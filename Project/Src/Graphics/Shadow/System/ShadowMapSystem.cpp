#include "ShadowMapSystem.h"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <limits>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/DepthStencil/DepthStencil.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "LGF/Graphics/Camera3D.h"
#include "LGF/Graphics/DirectionalLight.h"
#include "LGF/Graphics/Graphics3D.h"
#include "LGF/Math/MathConstants.h"
#include "LGF/Math/MathUtils.h"

using namespace LGF;

namespace {
	Vector4 NormalizePlane(const Vector4& plane) {
		const float length = std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
		assert(length > Math::EPSILON);
		return plane / length;
	}

	void ExtractFrustumPlanes(const Matrix4x4& matrix, Vector4 (&planes)[6]) {
		planes[0] = NormalizePlane({ matrix.m[0][3] + matrix.m[0][0], matrix.m[1][3] + matrix.m[1][0], matrix.m[2][3] + matrix.m[2][0], matrix.m[3][3] + matrix.m[3][0] });
		planes[1] = NormalizePlane({ matrix.m[0][3] - matrix.m[0][0], matrix.m[1][3] - matrix.m[1][0], matrix.m[2][3] - matrix.m[2][0], matrix.m[3][3] - matrix.m[3][0] });
		planes[2] = NormalizePlane({ matrix.m[0][3] + matrix.m[0][1], matrix.m[1][3] + matrix.m[1][1], matrix.m[2][3] + matrix.m[2][1], matrix.m[3][3] + matrix.m[3][1] });
		planes[3] = NormalizePlane({ matrix.m[0][3] - matrix.m[0][1], matrix.m[1][3] - matrix.m[1][1], matrix.m[2][3] - matrix.m[2][1], matrix.m[3][3] - matrix.m[3][1] });
		planes[4] = NormalizePlane({ matrix.m[0][2], matrix.m[1][2], matrix.m[2][2], matrix.m[3][2] });
		planes[5] = NormalizePlane({ matrix.m[0][3] - matrix.m[0][2], matrix.m[1][3] - matrix.m[1][2], matrix.m[2][3] - matrix.m[2][2], matrix.m[3][3] - matrix.m[3][2] });
	}

	float GetAspectRatio(const Camera3D& camera) {
		if (!camera.param.useWindowAspectRatio) {
			return camera.param.aspectRatio;
		}
		const uint32_t height = (std::max)(Graphics3D::GetWindowHeight(), 1u);
		return static_cast<float>(Graphics3D::GetWindowWidth()) / static_cast<float>(height);
	}
}

ShadowMapSystem::ShadowMapSystem(DirectXCore& directX) : directX_(directX) {}

ShadowMapSystem::~ShadowMapSystem() = default;

bool ShadowMapSystem::Initialize() {
	auto managers = directX_.GetResMgr();
	uint32_t firstSrvIndex = 0u;
	for (uint32_t cascadeIndex = 0;
		cascadeIndex < ShadowCascadeCalculator::kCascadeCount;
		++cascadeIndex) {
		depths_[cascadeIndex] = std::make_unique<DepthStencil>(
			*directX_.GetResourceFactory(),
			*managers.dsv,
			*managers.srvuav);
		depths_[cascadeIndex]->Initialize(
			kShadowMapSize,
			kShadowMapSize,
			DXGI_FORMAT_R32_TYPELESS,
			DXGI_FORMAT_D32_FLOAT,
			DXGI_FORMAT_R32_FLOAT);
		if (cascadeIndex == 0u) {
			firstSrvIndex = depths_[cascadeIndex]->GetSRVIndex();
		} else {
			assert(depths_[cascadeIndex]->GetSRVIndex() == firstSrvIndex + cascadeIndex);
		}
	}

	if (!cameraResources_.Initialize(
		*directX_.GetResourceFactory(),
		kCameraConstantBufferStride * ShadowCascadeCalculator::kCascadeCount)) {
		return false;
	}
	return cascadeDataResources_.Initialize(
		*directX_.GetResourceFactory(),
		sizeof(ShadowCascadeDataForGPU));
}

void ShadowMapSystem::Update(const Camera3D& camera, const DirectionalLight& light) {
	const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
	std::byte* cameraMappedData_ = cameraResources_.GetMappedData(frameIndex);
	ShadowCascadeDataForGPU* cascadeData_ =
		reinterpret_cast<ShadowCascadeDataForGPU*>(
			cascadeDataResources_.GetMappedData(frameIndex));
	constexpr float kBoundsPadding = 1.0f;
	constexpr float kCasterPadding = 50.0f;
	const float aspectRatio = GetAspectRatio(camera);
	cascadeFrusta_ = ShadowCascadeCalculator::Calculate(camera, aspectRatio);

	Vector3 forward = { 0.0f, -1.0f, 0.0f };
	if (Math::LengthSq(light.direction) > Math::EPSILON * Math::EPSILON) {
		forward = Math::Normalize(light.direction);
	}
	const Vector3 referenceUp = std::abs(forward.y) > 0.99f ? Vector3{ 0.0f, 0.0f, 1.0f } : Vector3{ 0.0f, 1.0f, 0.0f };
	const Vector3 right = Math::Normalize(Math::Cross(referenceUp, forward));
	const Vector3 up = Math::Cross(forward, right);

	const Quaternion cameraRotation = Math::MakeQuaternion(Vector3(camera.pitch, camera.yaw, 0.0f));
	const Matrix4x4 cameraWorld = Math::MakeAffineMatrix(
		{ 1.0f, 1.0f, 1.0f },
		cameraRotation,
		camera.translation);
	cascadeData_->view = Math::Inverse(cameraWorld);
	cascadeData_->splitDepths = {
		cascadeFrusta_[0].farClip,
		cascadeFrusta_[1].farClip,
		cascadeFrusta_[2].farClip,
		cascadeFrusta_[3].farClip,
	};
	cascadeData_->settings = {
		0.0015f,
		ShadowCascadeCalculator::kCascadeBlendRatio,
		(std::max)(camera.param.nearClip, 0.01f),
		0.0f,
	};

	for (uint32_t cascadeIndex = 0;
		cascadeIndex < ShadowCascadeCalculator::kCascadeCount;
		++cascadeIndex) {
		const std::array<Vector3, 8>& corners = cascadeFrusta_[cascadeIndex].corners;
		Vector3 center{};
		for (const Vector3& corner : corners) {
			center += corner;
		}
		center /= static_cast<float>(corners.size());

		float frustumRadius = 0.0f;
		for (const Vector3& corner : corners) {
			frustumRadius = (std::max)(frustumRadius, Math::Distance(center, corner));
		}
		const Vector3 position = center - forward * (frustumRadius + kCasterPadding);

		Matrix4x4 world{};
		world.m[0][0] = right.x; world.m[0][1] = right.y; world.m[0][2] = right.z;
		world.m[1][0] = up.x; world.m[1][1] = up.y; world.m[1][2] = up.z;
		world.m[2][0] = forward.x; world.m[2][1] = forward.y; world.m[2][2] = forward.z;
		world.m[3][0] = position.x; world.m[3][1] = position.y; world.m[3][2] = position.z; world.m[3][3] = 1.0f;

		const Matrix4x4 view = Math::Inverse(world);
		float minX = (std::numeric_limits<float>::max)();
		float maxX = (std::numeric_limits<float>::lowest)();
		float minY = (std::numeric_limits<float>::max)();
		float maxY = (std::numeric_limits<float>::lowest)();
		float minZ = (std::numeric_limits<float>::max)();
		float maxZ = (std::numeric_limits<float>::lowest)();
		for (const Vector3& corner : corners) {
			const Vector3 lightSpaceCorner = Math::Transform(corner, view);
			minX = (std::min)(minX, lightSpaceCorner.x);
			maxX = (std::max)(maxX, lightSpaceCorner.x);
			minY = (std::min)(minY, lightSpaceCorner.y);
			maxY = (std::max)(maxY, lightSpaceCorner.y);
			minZ = (std::min)(minZ, lightSpaceCorner.z);
			maxZ = (std::max)(maxZ, lightSpaceCorner.z);
		}

		const float nearClip = (std::max)(0.01f, minZ - kCasterPadding);
		const float farClip = (std::max)(nearClip + 0.01f, maxZ + kCasterPadding);
		Matrix4x4 projection = Math::MakeOrthographicMatrix(
			minX - kBoundsPadding,
			maxY + kBoundsPadding,
			maxX + kBoundsPadding,
			minY - kBoundsPadding,
			nearClip,
			farClip);
		Matrix4x4 viewProjection = view * projection;

		// ワールド原点のシャドウ座標をテクセル単位へ丸め、カメラ移動時の揺れを抑える。
		const Vector3 shadowOrigin = Math::Transform({ 0.0f, 0.0f, 0.0f }, viewProjection);
		const float texelScale = static_cast<float>(kShadowMapSize) * 0.5f;
		const float snappedOriginX = std::round(shadowOrigin.x * texelScale);
		const float snappedOriginY = std::round(shadowOrigin.y * texelScale);
		projection.m[3][0] += (snappedOriginX - shadowOrigin.x * texelScale) / texelScale;
		projection.m[3][1] += (snappedOriginY - shadowOrigin.y * texelScale) / texelScale;
		viewProjection = view * projection;

		Camera3DForGPU* cameraData = reinterpret_cast<Camera3DForGPU*>(
			cameraMappedData_ + kCameraConstantBufferStride * cascadeIndex);
		cameraData->viewProjection = viewProjection;
		cameraData->inverseViewProjection = Math::Inverse(cameraData->viewProjection);
		cameraData->worldPosition = position;
		cameraData->padding = cascadeData_->settings.x;
		ExtractFrustumPlanes(cameraData->viewProjection, cameraData->frustumPlanes);
		cascadeData_->viewProjections[cascadeIndex] = cameraData->viewProjection;
	}
}

void ShadowMapSystem::BeginShadowPass(uint32_t cascadeIndex) {
	assert(cascadeIndex < ShadowCascadeCalculator::kCascadeCount);
	currentCascadeIndex_ = cascadeIndex;
	ID3D12GraphicsCommandList* commandList = directX_.GetCommandList();
	depths_[cascadeIndex]->Transition(commandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	const D3D12_CPU_DESCRIPTOR_HANDLE dsv = depths_[cascadeIndex]->GetDSVHandleCPU();
	commandList->OMSetRenderTargets(0, nullptr, false, &dsv);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	const D3D12_VIEWPORT viewport{ 0.0f, 0.0f, float(kShadowMapSize), float(kShadowMapSize), 0.0f, 1.0f };
	const D3D12_RECT scissor{ 0, 0, LONG(kShadowMapSize), LONG(kShadowMapSize) };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);
}

void ShadowMapSystem::EndShadowPass() {
	depths_[currentCascadeIndex_]->Transition(
		directX_.GetCommandList(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	if (currentCascadeIndex_ + 1u == ShadowCascadeCalculator::kCascadeCount) {
		directX_.RestoreGBufferRenderTargets();
	}
}

void ShadowMapSystem::BindCamera(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIndex) const {
	commandList->SetGraphicsRootConstantBufferView(
		rootParamIndex,
		cameraResources_.GetGPUVirtualAddress(directX_.GetCurrentFrameContextIndex()) +
		kCameraConstantBufferStride * currentCascadeIndex_);
}

void ShadowMapSystem::BindCascadeData(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIndex) const {
	commandList->SetGraphicsRootConstantBufferView(
		rootParamIndex,
		cascadeDataResources_.GetGPUVirtualAddress(directX_.GetCurrentFrameContextIndex()));
}

D3D12_GPU_DESCRIPTOR_HANDLE ShadowMapSystem::GetShadowMapHandleGPU() const {
	return depths_.front()->GetSRVHandleGPU();
}
