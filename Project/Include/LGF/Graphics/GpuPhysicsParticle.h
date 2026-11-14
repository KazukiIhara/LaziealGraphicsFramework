#pragma once

#include "LGF/Math/MathTypes.h"

#include <cstdint>
#include <limits>
#include <span>
#include <vector>

namespace LGF {
	class Model;

	struct GpuPhysicsParticleWorldHandle {
		static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

		uint32_t index = InvalidIndex;
		uint32_t generation{};

		bool IsValid() const { return index != InvalidIndex && generation != 0u; }
		bool operator==(const GpuPhysicsParticleWorldHandle&) const = default;
	};

	enum class GpuPhysicsParticleCommandType : uint32_t {
		Add,
		Remove,
		SetPosition,
		SetVelocity,
		SetMass,
		SetRadius,
		SetMaterial,
		SetEnabled,
		AddForce,
		AddImpulse,
		SetRenderGroup,
	};

	struct GpuPhysicsParticleCommand {
		GpuPhysicsParticleCommandType type{};
		uint32_t index{};
		uint32_t generation{};
		uint32_t boolValue{};
		Vector3 vectorValue{};
		float scalarValue{};
		float restitution{};
		float friction{};
		uint32_t renderGroupIndex = std::numeric_limits<uint32_t>::max();
		uint32_t padding{};
	};
	static_assert(sizeof(GpuPhysicsParticleCommand) == 48u);

	enum class GpuPhysicsColliderType : uint32_t {
		Sphere,
		SphericalCap,
		SphericalShellCap,
		AABB,
		OBB,
	};

	// Runtime側のCollision型へ依存せず、全対応形状をGPUへ渡すための共通表現。
	struct GpuPhysicsCollider {
		GpuPhysicsColliderType type{};
		uint32_t padding0[3]{};
		Vector4 data0{};
		Vector4 data1{};
		Vector4 data2{};
		Vector4 data3{};
		Vector3 referencePosition{};
		float restitution{};
		Vector3 linearVelocity{};
		float friction{};
		Vector3 angularVelocity{};
		float padding1{};
	};
	static_assert(sizeof(GpuPhysicsCollider) == 128u);

	struct GpuPhysicsParticleWorldDesc {
		Vector3 worldMinimum{ -100.0f, -100.0f, -100.0f };
		Vector3 worldMaximum{ 100.0f, 100.0f, 100.0f };
		Vector3 gravity{ 0.0f, -9.8f, 0.0f };
		uint32_t maximumParticleCount = 65535u;
		uint32_t maximumColliderCount = 4096u;
		uint32_t maximumCommandCount = 262144u;
		uint32_t substepCount = 4u;
		uint32_t solverIterationCount = 4u;
		float maximumParticleRadius = 1.0f;
		float linearDamping = 0.1f;
		float penetrationSlop = 0.005f;
		float positionCorrectionRate = 0.2f;
		float maximumDepenetrationVelocity = 5.0f;
		float bounceVelocityThreshold = 0.8f;
	};

	struct GpuPhysicsParticleStatistics {
		uint32_t particleCount{};
		uint32_t candidatePairCount{};
		uint32_t retiredParticleCount{};
		uint32_t latencyFrames{};
		bool isAvailable{};
	};

	struct GpuRetiredPhysicsParticle {
		uint32_t index{};
		uint32_t generation{};
	};

	struct GpuPhysicsParticleState {
		Vector3 position{};
		float radius{};
		Vector3 previousPosition{};
		float inverseMass{};
		Vector3 velocity{};
		uint32_t generation{};
		Vector3 force{};
		uint32_t alive{};
		float restitution{};
		float friction{};
		uint32_t enabled{};
		uint32_t renderGroupIndex = std::numeric_limits<uint32_t>::max();
	};
	static_assert(sizeof(GpuPhysicsParticleState) == 80u);

	namespace GpuCompute {
		GpuPhysicsParticleWorldHandle CreatePhysicsParticleWorld(const GpuPhysicsParticleWorldDesc& desc);
		bool RemovePhysicsParticleWorld(GpuPhysicsParticleWorldHandle handle);
		bool ClearPhysicsParticleWorld(GpuPhysicsParticleWorldHandle handle);
		bool SetPhysicsParticleWorldSettings(
			GpuPhysicsParticleWorldHandle handle,
			const GpuPhysicsParticleWorldDesc& desc);
		bool UpdatePhysicsParticleWorld(
			GpuPhysicsParticleWorldHandle handle,
			float deltaTime,
			std::span<const GpuPhysicsParticleCommand> commands,
			std::span<const GpuPhysicsCollider> colliders,
			uint32_t colliderCountPerSubstep);
		GpuPhysicsParticleStatistics GetPhysicsParticleStatistics(GpuPhysicsParticleWorldHandle handle);
		bool ConsumeRetiredPhysicsParticles(
			GpuPhysicsParticleWorldHandle handle,
			std::vector<GpuRetiredPhysicsParticle>& particles);
		bool RequestPhysicsParticleStates(GpuPhysicsParticleWorldHandle handle);
		bool GetLatestPhysicsParticleStates(
			GpuPhysicsParticleWorldHandle handle,
			std::vector<GpuPhysicsParticleState>& states,
			uint32_t& latencyFrames);
		bool GetLatestPhysicsParticleState(
			GpuPhysicsParticleWorldHandle handle,
			uint32_t particleIndex,
			GpuPhysicsParticleState& state,
			uint32_t& latencyFrames);
		bool DrawPhysicsParticleSpheres(
			GpuPhysicsParticleWorldHandle handle,
			uint32_t renderGroupIndex,
			const Vector4& color);
		bool DrawPhysicsParticleModel(
			GpuPhysicsParticleWorldHandle handle,
			uint32_t renderGroupIndex,
			const Model& model,
			const Vector3& scale,
			const Vector4& color);
	}

}
