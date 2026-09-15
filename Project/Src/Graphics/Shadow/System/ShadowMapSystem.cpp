#include "ShadowMapSystem.h"

#include <cassert>
#include <cmath>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/DepthStencil/DepthStencil.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "LGF/Graphics/DirectionalLight.h"
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
}

ShadowMapSystem::ShadowMapSystem(DirectXCore& directX) : directX_(directX) {}

ShadowMapSystem::~ShadowMapSystem() {
	if (cameraResource_ && cameraData_) {
		cameraResource_->Unmap(0, nullptr);
	}
}

bool ShadowMapSystem::Initialize() {
	auto managers = directX_.GetResMgr();
	depth_ = std::make_unique<DepthStencil>(*directX_.GetResourceFactory(), *managers.dsv, *managers.srvuav);
	depth_->Initialize(kShadowMapSize, kShadowMapSize, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);
	cameraResource_ = directX_.GetResourceFactory()->CreateUploadBufferResource(sizeof(Camera3DForGPU));
	if (!cameraResource_) {
		return false;
	}
	return SUCCEEDED(cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_))) && cameraData_;
}

void ShadowMapSystem::Update(const DirectionalLight& light) {
	Vector3 forward = Math::Normalize(light.direction);
	if (Math::Length(forward) <= Math::EPSILON) {
		forward = { 0.0f, -1.0f, 0.0f };
	}
	const Vector3 referenceUp = std::abs(forward.y) > 0.99f ? Vector3{ 0.0f, 0.0f, 1.0f } : Vector3{ 0.0f, 1.0f, 0.0f };
	const Vector3 right = Math::Normalize(Math::Cross(referenceUp, forward));
	const Vector3 up = Math::Cross(forward, right);
	const Vector3 position = forward * -30.0f;

	Matrix4x4 world{};
	world.m[0][0] = right.x; world.m[0][1] = right.y; world.m[0][2] = right.z;
	world.m[1][0] = up.x; world.m[1][1] = up.y; world.m[1][2] = up.z;
	world.m[2][0] = forward.x; world.m[2][1] = forward.y; world.m[2][2] = forward.z;
	world.m[3][0] = position.x; world.m[3][1] = position.y; world.m[3][2] = position.z; world.m[3][3] = 1.0f;

	const Matrix4x4 projection = Math::MakeOrthographicMatrix(-15.0f, 15.0f, 15.0f, -15.0f, 0.1f, 60.0f);
	cameraData_->viewProjection = Math::Inverse(world) * projection;
	cameraData_->inverseViewProjection = Math::Inverse(cameraData_->viewProjection);
	cameraData_->worldPosition = position;
	cameraData_->padding = 0.0015f;
	ExtractFrustumPlanes(cameraData_->viewProjection, cameraData_->frustumPlanes);
}

void ShadowMapSystem::BeginShadowPass() {
	ID3D12GraphicsCommandList* commandList = directX_.GetCommandList();
	depth_->Transition(commandList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	const D3D12_CPU_DESCRIPTOR_HANDLE dsv = depth_->GetDSVHandleCPU();
	commandList->OMSetRenderTargets(0, nullptr, false, &dsv);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	const D3D12_VIEWPORT viewport{ 0.0f, 0.0f, float(kShadowMapSize), float(kShadowMapSize), 0.0f, 1.0f };
	const D3D12_RECT scissor{ 0, 0, LONG(kShadowMapSize), LONG(kShadowMapSize) };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);
}

void ShadowMapSystem::EndShadowPass() {
	depth_->Transition(directX_.GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	directX_.RestoreGBufferRenderTargets();
}

void ShadowMapSystem::BindCamera(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIndex) const {
	commandList->SetGraphicsRootConstantBufferView(rootParamIndex, cameraResource_->GetGPUVirtualAddress());
}

D3D12_GPU_DESCRIPTOR_HANDLE ShadowMapSystem::GetShadowMapHandleGPU() const {
	return depth_->GetSRVHandleGPU();
}
