#include "EffekseerSystem.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>
#include <vector>

#include <Effekseer.h>
#include <EffekseerRendererDX12.h>

#include "Core/Logger/Logger.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "LGF/Graphics/Camera3D.h"
#include "LGF/Math/Functions/MatrixMath.h"
#include "LGF/Math/Functions/ProjectionMath.h"
#include "LGF/Math/Functions/QuaternionMath.h"
#include "LGF/Math/Functions/TransformMath.h"

namespace {

	constexpr uint32_t kMaximumInstanceCount = 10000u;
	constexpr float kAffineEpsilon = 0.0001f;

	uint32_t NextGeneration(uint32_t generation) {
		++generation;
		return generation == 0u ? 1u : generation;
	}

	bool IsAffineTransform(const LGF::Matrix4x4& transform) {
		for (const auto& row : transform.m) {
			for (float value : row) {
				if (!std::isfinite(value)) {
					return false;
				}
			}
		}
		return std::fabs(transform.m[0][3]) <= kAffineEpsilon &&
			std::fabs(transform.m[1][3]) <= kAffineEpsilon &&
			std::fabs(transform.m[2][3]) <= kAffineEpsilon &&
			std::fabs(transform.m[3][3] - 1.0f) <= kAffineEpsilon;
	}

	Effekseer::Matrix43 ToEffekseerMatrix43(const LGF::Matrix4x4& source) {
		Effekseer::Matrix43 result{};
		for (uint32_t row = 0u; row < 4u; ++row) {
			for (uint32_t column = 0u; column < 3u; ++column) {
				result.Value[row][column] = source.m[row][column];
			}
		}
		return result;
	}

	Effekseer::Matrix44 ToEffekseerMatrix44(const LGF::Matrix4x4& source) {
		Effekseer::Matrix44 result;
		for (uint32_t row = 0u; row < 4u; ++row) {
			for (uint32_t column = 0u; column < 4u; ++column) {
				result.Values[row][column] = source.m[row][column];
			}
		}
		return result;
	}

}

class LGF::EffekseerSystem::Impl {
public:
	struct ResourceSlot {
		Effekseer::EffectRef effect;
		std::filesystem::path filePath;
		uint32_t generation = 1u;
	};

	struct InstanceSlot {
		Effekseer::Handle handle = -1;
		uint32_t generation = 1u;
		bool isOccupied{};
	};

	struct World {
		Effekseer::ManagerRef manager;
		std::vector<ResourceSlot> resources;
		std::vector<uint32_t> freeResourceIndices;
		std::vector<InstanceSlot> instances;
		std::vector<uint32_t> freeInstanceIndices;
		float elapsedTime{};
	};

	struct WorldSlot {
		std::unique_ptr<World> world;
		uint32_t generation = 1u;
	};

	World* FindWorld(EffectWorldHandle3D handle) {
		return const_cast<World*>(std::as_const(*this).FindWorld(handle));
	}

	const World* FindWorld(EffectWorldHandle3D handle) const {
		if (!handle.IsValid() || handle.index >= worlds.size()) {
			return nullptr;
		}
		const WorldSlot& slot = worlds[handle.index];
		return slot.generation == handle.generation ? slot.world.get() : nullptr;
	}

	ResourceSlot* FindResource(EffectResourceHandle3D handle) {
		return const_cast<ResourceSlot*>(std::as_const(*this).FindResource(handle));
	}

	const ResourceSlot* FindResource(EffectResourceHandle3D handle) const {
		const World* world = FindWorld(handle.world);
		if (!world || !handle.IsValid() || handle.index >= world->resources.size()) {
			return nullptr;
		}
		const ResourceSlot& slot = world->resources[handle.index];
		return slot.generation == handle.generation && slot.effect ? &slot : nullptr;
	}

	const InstanceSlot* FindInstance(EffectInstanceHandle3D handle) const {
		const World* world = FindWorld(handle.world);
		if (!world || !handle.IsValid() || handle.index >= world->instances.size()) {
			return nullptr;
		}
		const InstanceSlot& slot = world->instances[handle.index];
		return slot.generation == handle.generation && slot.isOccupied ? &slot : nullptr;
	}

	InstanceSlot* FindInstance(EffectInstanceHandle3D handle) {
		return const_cast<InstanceSlot*>(std::as_const(*this).FindInstance(handle));
	}

	void ReleaseInstance(World& world, uint32_t index) {
		InstanceSlot& slot = world.instances[index];
		if (!slot.isOccupied) {
			return;
		}
		slot.handle = -1;
		slot.isOccupied = false;
		slot.generation = NextGeneration(slot.generation);
		world.freeInstanceIndices.push_back(index);
	}

	Effekseer::Backend::GraphicsDeviceRef graphicsDevice;
	EffekseerRenderer::RendererRef renderer;
	Effekseer::RefPtr<EffekseerRenderer::SingleFrameMemoryPool> memoryPool;
	Effekseer::RefPtr<EffekseerRenderer::CommandList> commandList;
	std::vector<WorldSlot> worlds;
	std::vector<uint32_t> freeWorldIndices;
	bool isFrameActive{};
};

LGF::EffekseerSystem::EffekseerSystem(
	DirectXCore& directX,
	Camera3DSystem& cameraSystem) :
	directX_(directX),
	cameraSystem_(cameraSystem),
	impl_(std::make_unique<Impl>()) {
}

LGF::EffekseerSystem::~EffekseerSystem() = default;

bool LGF::EffekseerSystem::Initialize() {
	impl_->graphicsDevice = EffekseerRendererDX12::CreateGraphicsDevice(
		directX_.GetDevice(),
		directX_.GetCommandQueue(),
		static_cast<int32_t>(directX_.GetFrameContextCountInUse()));
	if (!impl_->graphicsDevice) {
		return false;
	}

	DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	impl_->renderer = EffekseerRendererDX12::Create(
		impl_->graphicsDevice,
		&renderTargetFormat,
		1,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		false,
		static_cast<int32_t>(kMaximumInstanceCount));
	if (!impl_->renderer) {
		return false;
	}
	impl_->renderer->SetMaintainGammaColorInLinearColorSpace(true);

	impl_->memoryPool = EffekseerRenderer::CreateSingleFrameMemoryPool(
		impl_->renderer->GetGraphicsDevice());
	if (!impl_->memoryPool) {
		return false;
	}
	impl_->commandList = EffekseerRenderer::CreateCommandList(
		impl_->renderer->GetGraphicsDevice(),
		impl_->memoryPool);
	return impl_->commandList != nullptr;
}

void LGF::EffekseerSystem::BeginFrame() {
	impl_->memoryPool->NewFrame();
	impl_->isFrameActive = true;
}

void LGF::EffekseerSystem::EndFrame() {
	impl_->isFrameActive = false;
}

void LGF::EffekseerSystem::Finalize() {
	impl_->worlds.clear();
	impl_->freeWorldIndices.clear();
	impl_->commandList.Reset();
	impl_->memoryPool.Reset();
	impl_->renderer.Reset();
	impl_->graphicsDevice.Reset();
	impl_->isFrameActive = false;
}

LGF::EffectWorldHandle3D LGF::EffekseerSystem::CreateWorld(
	uint32_t maximumInstanceCount) {
	if (maximumInstanceCount == 0u || maximumInstanceCount > kMaximumInstanceCount) {
		return {};
	}

	auto world = std::make_unique<Impl::World>();
	world->manager = Effekseer::Manager::Create(
		static_cast<int32_t>(maximumInstanceCount));
	if (!world->manager) {
		return {};
	}
	world->manager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
	world->manager->SetSpriteRenderer(impl_->renderer->CreateSpriteRenderer());
	world->manager->SetRibbonRenderer(impl_->renderer->CreateRibbonRenderer());
	world->manager->SetRingRenderer(impl_->renderer->CreateRingRenderer());
	world->manager->SetTrackRenderer(impl_->renderer->CreateTrackRenderer());
	world->manager->SetModelRenderer(impl_->renderer->CreateModelRenderer());
	world->manager->SetTextureLoader(impl_->renderer->CreateTextureLoader());
	world->manager->SetModelLoader(impl_->renderer->CreateModelLoader());
	world->manager->SetMaterialLoader(impl_->renderer->CreateMaterialLoader());
	world->manager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());

	uint32_t index = 0u;
	if (!impl_->freeWorldIndices.empty()) {
		index = impl_->freeWorldIndices.back();
		impl_->freeWorldIndices.pop_back();
		impl_->worlds[index].world = std::move(world);
	} else {
		index = static_cast<uint32_t>(impl_->worlds.size());
		impl_->worlds.push_back({ .world = std::move(world) });
	}
	return { index, impl_->worlds[index].generation };
}

bool LGF::EffekseerSystem::RemoveWorld(EffectWorldHandle3D handle) {
	Impl::World* world = impl_->FindWorld(handle);
	if (!world) {
		return false;
	}
	world->manager->StopAllEffects();
	Impl::WorldSlot& slot = impl_->worlds[handle.index];
	slot.world.reset();
	slot.generation = NextGeneration(slot.generation);
	impl_->freeWorldIndices.push_back(handle.index);
	return true;
}

LGF::EffectResourceHandle3D LGF::EffekseerSystem::Load(
	EffectWorldHandle3D worldHandle,
	const std::filesystem::path& filePath) {
	Impl::World* world = impl_->FindWorld(worldHandle);
	if (!world || filePath.empty()) {
		return {};
	}

	const std::filesystem::path resolvedPath =
		std::filesystem::absolute(filePath).lexically_normal();
	const std::u16string path = resolvedPath.u16string();
	Effekseer::EffectRef effect = Effekseer::Effect::Create(
		world->manager,
		path.c_str());
	if (!effect) {
		Logger::Log("Failed to load Effekseer effect: " + resolvedPath.string());
		return {};
	}

	uint32_t index = 0u;
	if (!world->freeResourceIndices.empty()) {
		index = world->freeResourceIndices.back();
		world->freeResourceIndices.pop_back();
		Impl::ResourceSlot& slot = world->resources[index];
		slot.effect = std::move(effect);
		slot.filePath = resolvedPath;
	} else {
		index = static_cast<uint32_t>(world->resources.size());
		world->resources.push_back({
			.effect = std::move(effect),
			.filePath = resolvedPath,
		});
	}
	return { index, world->resources[index].generation, worldHandle };
}

bool LGF::EffekseerSystem::IsValid(EffectResourceHandle3D handle) const {
	return impl_->FindResource(handle) != nullptr;
}

bool LGF::EffekseerSystem::Remove(EffectResourceHandle3D handle) {
	Impl::ResourceSlot* resource = impl_->FindResource(handle);
	Impl::World* world = impl_->FindWorld(handle.world);
	if (!resource || !world) {
		return false;
	}
	resource->effect.Reset();
	resource->filePath.clear();
	resource->generation = NextGeneration(resource->generation);
	world->freeResourceIndices.push_back(handle.index);
	return true;
}

LGF::EffectInstanceHandle3D LGF::EffekseerSystem::Play(
	EffectResourceHandle3D effectHandle,
	const Matrix4x4& transform) {
	Impl::ResourceSlot* resource = impl_->FindResource(effectHandle);
	Impl::World* world = impl_->FindWorld(effectHandle.world);
	if (!resource || !world || !IsAffineTransform(transform)) {
		return {};
	}

	const Effekseer::Handle handle = world->manager->Play(resource->effect, 0.0f, 0.0f, 0.0f);
	if (handle < 0) {
		return {};
	}
	world->manager->SetMatrix(handle, ToEffekseerMatrix43(transform));

	uint32_t index = 0u;
	if (!world->freeInstanceIndices.empty()) {
		index = world->freeInstanceIndices.back();
		world->freeInstanceIndices.pop_back();
		Impl::InstanceSlot& slot = world->instances[index];
		slot.handle = handle;
		slot.isOccupied = true;
	} else {
		index = static_cast<uint32_t>(world->instances.size());
		world->instances.push_back({
			.handle = handle,
			.isOccupied = true,
		});
	}
	return { index, world->instances[index].generation, effectHandle.world };
}

bool LGF::EffekseerSystem::Stop(EffectInstanceHandle3D handle) {
	Impl::InstanceSlot* instance = impl_->FindInstance(handle);
	Impl::World* world = impl_->FindWorld(handle.world);
	if (!instance || !world) {
		return false;
	}
	world->manager->StopEffect(instance->handle);
	impl_->ReleaseInstance(*world, handle.index);
	return true;
}

bool LGF::EffekseerSystem::IsPlaying(EffectInstanceHandle3D handle) const {
	const Impl::InstanceSlot* instance = impl_->FindInstance(handle);
	const Impl::World* world = impl_->FindWorld(handle.world);
	return instance && world && world->manager->Exists(instance->handle);
}

bool LGF::EffekseerSystem::SetTransform(
	EffectInstanceHandle3D handle,
	const Matrix4x4& transform) {
	Impl::InstanceSlot* instance = impl_->FindInstance(handle);
	Impl::World* world = impl_->FindWorld(handle.world);
	if (!instance || !world || !IsAffineTransform(transform) ||
		!world->manager->Exists(instance->handle)) {
		return false;
	}
	world->manager->SetMatrix(instance->handle, ToEffekseerMatrix43(transform));
	return true;
}

bool LGF::EffekseerSystem::Update(EffectWorldHandle3D handle, float deltaTime) {
	Impl::World* world = impl_->FindWorld(handle);
	if (!world || !std::isfinite(deltaTime) || deltaTime < 0.0f) {
		return false;
	}

	Effekseer::Manager::UpdateParameter parameter{};
	parameter.DeltaFrame = deltaTime * 60.0f;
	parameter.SyncUpdate = true;
	world->manager->Update(parameter);
	world->elapsedTime += deltaTime;

	for (uint32_t index = 0u; index < world->instances.size(); ++index) {
		const Impl::InstanceSlot& instance = world->instances[index];
		if (instance.isOccupied && !world->manager->Exists(instance.handle)) {
			impl_->ReleaseInstance(*world, index);
		}
	}
	return true;
}

bool LGF::EffekseerSystem::Clear(EffectWorldHandle3D handle) {
	Impl::World* world = impl_->FindWorld(handle);
	if (!world) {
		return false;
	}
	world->manager->StopAllEffects();
	for (uint32_t index = 0u; index < world->instances.size(); ++index) {
		impl_->ReleaseInstance(*world, index);
	}
	return true;
}

void LGF::EffekseerSystem::Draw() {
	if (!impl_->isFrameActive || impl_->worlds.empty()) {
		return;
	}

	const Camera3D& camera = cameraSystem_.GetCamera();
	const Quaternion rotation = Math::MakeQuaternion(
		Vector3(camera.pitch, camera.yaw, 0.0f));
	const Matrix4x4 cameraWorld = Math::MakeAffineMatrix(
		{ 1.0f, 1.0f, 1.0f },
		rotation,
		camera.translation);
	const Vector3 cameraFront = Math::TransformNormal(
		{ 0.0f, 0.0f, 1.0f },
		cameraWorld);
	const Matrix4x4 view = Math::Inverse(cameraWorld);
	const float aspectRatio = camera.param.useWindowAspectRatio
		? static_cast<float>(directX_.GetRenderWidth()) /
			static_cast<float>(directX_.GetRenderHeight())
		: camera.param.aspectRatio;
	const Matrix4x4 projection = Math::MakePerspectiveFovMatrix(
		camera.param.fovY,
		aspectRatio,
		camera.param.nearClip,
		camera.param.farClip);

	EffekseerRendererDX12::BeginCommandList(
		impl_->commandList,
		directX_.GetCommandList());
	impl_->renderer->SetCommandList(impl_->commandList);
	impl_->renderer->SetCameraMatrix(ToEffekseerMatrix44(view));
	impl_->renderer->SetProjectionMatrix(ToEffekseerMatrix44(projection));

	for (Impl::WorldSlot& slot : impl_->worlds) {
		if (!slot.world) {
			continue;
		}
		Impl::World& world = *slot.world;
		Effekseer::Manager::LayerParameter layerParameter{};
		layerParameter.ViewerPosition = {
			camera.translation.x,
			camera.translation.y,
			camera.translation.z,
		};
		world.manager->SetLayerParameter(0, layerParameter);
		impl_->renderer->SetTime(world.elapsedTime);
		if (!impl_->renderer->BeginRendering()) {
			continue;
		}

		Effekseer::Manager::DrawParameter drawParameter{};
		drawParameter.ZNear = 0.0f;
		drawParameter.ZFar = 1.0f;
		drawParameter.ViewProjectionMatrix =
			impl_->renderer->GetCameraProjectionMatrix();
		drawParameter.CameraPosition = {
			camera.translation.x,
			camera.translation.y,
			camera.translation.z,
		};
		drawParameter.CameraFrontDirection = {
			cameraFront.x,
			cameraFront.y,
			cameraFront.z,
		};
		drawParameter.IsSortingEffectsEnabled = true;
		world.manager->Draw(drawParameter);
		impl_->renderer->EndRendering();
	}

	impl_->renderer->SetCommandList(nullptr);
	EffekseerRendererDX12::EndCommandList(impl_->commandList);

	ID3D12DescriptorHeap* heaps[] = {
		directX_.GetResMgr().srvuav->GetDescriptorHeap()
	};
	directX_.GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
	directX_.SetViewportAndScissor(
		directX_.GetRenderWidth(),
		directX_.GetRenderHeight());
}
