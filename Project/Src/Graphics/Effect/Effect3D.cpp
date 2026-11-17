#include "LGF/Graphics/Effect3D.h"

#include <cassert>

#include "Effect3DInternal.h"
#include "EffekseerSystem.h"

namespace {
	LGF::EffekseerSystem* g_effekseerSystem = nullptr;
}

bool LGF::EffectWorldHandle3D::IsValid() const {
	return index != InvalidIndex && generation != 0u;
}

bool LGF::EffectResourceHandle3D::IsValid() const {
	return index != InvalidIndex && generation != 0u && world.IsValid();
}

bool LGF::EffectInstanceHandle3D::IsValid() const {
	return index != InvalidIndex && generation != 0u && world.IsValid();
}

void LGF::Effect3D::Internal::SetEffekseerSystem(EffekseerSystem* system) {
	assert(system != nullptr);
	g_effekseerSystem = system;
}

void LGF::Effect3D::Internal::ResetEffekseerSystem() {
	g_effekseerSystem = nullptr;
}

LGF::EffectWorldHandle3D LGF::Effect3D::CreateWorld(uint32_t maximumInstanceCount) {
	return g_effekseerSystem
		? g_effekseerSystem->CreateWorld(maximumInstanceCount)
		: EffectWorldHandle3D{};
}

bool LGF::Effect3D::RemoveWorld(EffectWorldHandle3D handle) {
	return g_effekseerSystem && g_effekseerSystem->RemoveWorld(handle);
}

LGF::EffectResourceHandle3D LGF::Effect3D::Load(
	EffectWorldHandle3D world,
	const std::filesystem::path& filePath) {
	return g_effekseerSystem
		? g_effekseerSystem->Load(world, filePath)
		: EffectResourceHandle3D{};
}

bool LGF::Effect3D::IsValid(EffectResourceHandle3D handle) {
	return g_effekseerSystem && g_effekseerSystem->IsValid(handle);
}

bool LGF::Effect3D::Remove(EffectResourceHandle3D handle) {
	return g_effekseerSystem && g_effekseerSystem->Remove(handle);
}

LGF::EffectInstanceHandle3D LGF::Effect3D::Play(
	EffectResourceHandle3D effect,
	const Matrix4x4& transform) {
	return g_effekseerSystem
		? g_effekseerSystem->Play(effect, transform)
		: EffectInstanceHandle3D{};
}

bool LGF::Effect3D::Stop(EffectInstanceHandle3D handle) {
	return g_effekseerSystem && g_effekseerSystem->Stop(handle);
}

bool LGF::Effect3D::IsPlaying(EffectInstanceHandle3D handle) {
	return g_effekseerSystem && g_effekseerSystem->IsPlaying(handle);
}

bool LGF::Effect3D::SetTransform(
	EffectInstanceHandle3D handle,
	const Matrix4x4& transform) {
	return g_effekseerSystem && g_effekseerSystem->SetTransform(handle, transform);
}

bool LGF::Effect3D::Update(EffectWorldHandle3D handle, float deltaTime) {
	return g_effekseerSystem && g_effekseerSystem->Update(handle, deltaTime);
}

bool LGF::Effect3D::Clear(EffectWorldHandle3D handle) {
	return g_effekseerSystem && g_effekseerSystem->Clear(handle);
}
