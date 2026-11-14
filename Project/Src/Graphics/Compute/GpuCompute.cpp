#include "LGF/Graphics/GpuCompute.h"

#include <cassert>

#include "GpuComputeInternal.h"
#include "PhysicsParticle/GpuPhysicsParticleBackend.h"

namespace {
	LGF::GpuPhysicsParticleBackend* g_physicsParticleBackend = nullptr;
}

void LGF::GpuCompute::Internal::SetPhysicsParticleBackend(GpuPhysicsParticleBackend* backend) {
	assert(backend != nullptr);
	g_physicsParticleBackend = backend;
}

void LGF::GpuCompute::Internal::ResetPhysicsParticleBackend() {
	g_physicsParticleBackend = nullptr;
}

LGF::GpuPhysicsParticleWorldHandle LGF::GpuCompute::CreatePhysicsParticleWorld(
	const GpuPhysicsParticleWorldDesc& desc) {
	return g_physicsParticleBackend
		? g_physicsParticleBackend->CreateWorld(desc)
		: GpuPhysicsParticleWorldHandle{};
}

bool LGF::GpuCompute::RemovePhysicsParticleWorld(GpuPhysicsParticleWorldHandle handle) {
	return g_physicsParticleBackend && g_physicsParticleBackend->RemoveWorld(handle);
}

bool LGF::GpuCompute::ClearPhysicsParticleWorld(GpuPhysicsParticleWorldHandle handle) {
	return g_physicsParticleBackend && g_physicsParticleBackend->ClearWorld(handle);
}

bool LGF::GpuCompute::SetPhysicsParticleWorldSettings(
	GpuPhysicsParticleWorldHandle handle,
	const GpuPhysicsParticleWorldDesc& desc) {
	return g_physicsParticleBackend &&
		g_physicsParticleBackend->SetWorldSettings(handle, desc);
}

bool LGF::GpuCompute::UpdatePhysicsParticleWorld(
	GpuPhysicsParticleWorldHandle handle,
	float deltaTime,
	std::span<const GpuPhysicsParticleCommand> commands,
	std::span<const GpuPhysicsCollider> colliders,
	uint32_t colliderCountPerSubstep) {
	return g_physicsParticleBackend && g_physicsParticleBackend->UpdateWorld(
		handle,
		deltaTime,
		commands,
		colliders,
		colliderCountPerSubstep);
}

LGF::GpuPhysicsParticleStatistics LGF::GpuCompute::GetPhysicsParticleStatistics(
	GpuPhysicsParticleWorldHandle handle) {
	return g_physicsParticleBackend
		? g_physicsParticleBackend->GetStatistics(handle)
		: GpuPhysicsParticleStatistics{};
}

bool LGF::GpuCompute::ConsumeRetiredPhysicsParticles(
	GpuPhysicsParticleWorldHandle handle,
	std::vector<GpuRetiredPhysicsParticle>& particles) {
	return g_physicsParticleBackend &&
		g_physicsParticleBackend->ConsumeRetiredParticles(handle, particles);
}

bool LGF::GpuCompute::RequestPhysicsParticleStates(GpuPhysicsParticleWorldHandle handle) {
	return g_physicsParticleBackend && g_physicsParticleBackend->RequestStates(handle);
}

bool LGF::GpuCompute::GetLatestPhysicsParticleStates(
	GpuPhysicsParticleWorldHandle handle,
	std::vector<GpuPhysicsParticleState>& states,
	uint32_t& latencyFrames) {
	return g_physicsParticleBackend &&
		g_physicsParticleBackend->GetLatestStates(handle, states, latencyFrames);
}

bool LGF::GpuCompute::GetLatestPhysicsParticleState(
	GpuPhysicsParticleWorldHandle handle,
	uint32_t particleIndex,
	GpuPhysicsParticleState& state,
	uint32_t& latencyFrames) {
	return g_physicsParticleBackend && g_physicsParticleBackend->GetLatestState(
		handle,
		particleIndex,
		state,
		latencyFrames);
}

bool LGF::GpuCompute::DrawPhysicsParticleSpheres(
	GpuPhysicsParticleWorldHandle handle,
	uint32_t renderGroupIndex,
	const Vector4& color) {
	return g_physicsParticleBackend &&
		g_physicsParticleBackend->DrawSpheres(handle, renderGroupIndex, color);
}

bool LGF::GpuCompute::DrawPhysicsParticleModel(
	GpuPhysicsParticleWorldHandle handle,
	uint32_t renderGroupIndex,
	const Model& model,
	const Vector3& scale,
	const Vector4& color) {
	return g_physicsParticleBackend && g_physicsParticleBackend->DrawModel(
		handle, renderGroupIndex, model, scale, color);
}
