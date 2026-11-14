#pragma once

#include "Graphics/DirectX/ComPtr/ComPtr.h"
#include "Graphics/DirectX/FrameContext/FrameContext.h"
#include "Graphics/System/IGraphicsSubsystem.h"
#include "LGF/Graphics/GpuPhysicsParticle.h"

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace LGF {

	class DirectXCore;
	class Camera3DSystem;
	class Model;
	struct ModelResource;
	class Pipeline;
	class PipelineManager;

	class GpuPhysicsParticleBackend : public IGraphicsSubsystem {
	private:
		struct ParticleForGPU {
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
			uint32_t renderGroupIndex = UINT32_MAX;
		};
		static_assert(sizeof(ParticleForGPU) == 80u);

		struct ConstantsForGPU {
			Vector3 worldMinimum{};
			float deltaTime{};
			Vector3 worldMaximum{};
			float damping{};
			Vector3 gravity{};
			float maximumCorrection{};
			uint32_t maximumParticleCount{};
			uint32_t hashBucketCount{};
			uint32_t colliderCount{};
			uint32_t commandCount{};
			uint32_t substepIndex{};
			uint32_t solverIterationCount{};
			uint32_t commandRangeCount{};
			uint32_t padding0{};
			float cellSize{};
			float penetrationSlop{};
			float positionCorrectionRate{};
			float bounceVelocityThreshold{};
		};
		static_assert(sizeof(ConstantsForGPU) == 96u);

		struct CountersForGPU {
			uint32_t aliveCount{};
			uint32_t candidatePairCount{};
			uint32_t retiredCount{};
			uint32_t padding{};
		};
		static_assert(sizeof(CountersForGPU) == 16u);

		struct CommandRangeForGPU {
			uint32_t firstCommand{};
			uint32_t commandCount{};
		};
		static_assert(sizeof(CommandRangeForGPU) == 8u);

		struct World {
			struct SphereDrawRequest {
				uint32_t renderGroupIndex{};
				Vector4 color{};
			};
			struct ModelDrawRequest {
				uint32_t renderGroupIndex{};
				std::shared_ptr<ModelResource> resource;
				Vector3 scale{ 1.0f, 1.0f, 1.0f };
				Vector4 color{};
			};

			GpuPhysicsParticleWorldDesc desc{};
			uint32_t hashBucketCount{};
			ComPtr<ID3D12Resource> particleResource;
			ComPtr<ID3D12Resource> correctionResource;
			ComPtr<ID3D12Resource> bucketHeadResource;
			ComPtr<ID3D12Resource> particleCellResource;
			ComPtr<ID3D12Resource> particleNextResource;
			ComPtr<ID3D12Resource> counterResource;
			ComPtr<ID3D12Resource> retiredResource;
			FrameBufferedUploadBuffer commandResources;
			FrameBufferedUploadBuffer commandRangeResources;
			FrameBufferedUploadBuffer colliderResources;
			FrameBufferedUploadBuffer constantResources;
			FrameBufferedUploadBuffer drawConstantResources;
			FrameBufferedUploadBuffer modelDrawConstantResources;
			std::array<ComPtr<ID3D12Resource>, kFrameContextCount> readbackResources{};
			std::array<ComPtr<ID3D12Resource>, kFrameContextCount> stateReadbackResources{};
			std::array<uint32_t, kFrameContextCount> commandSrvIndices{};
			std::array<uint32_t, kFrameContextCount> commandRangeSrvIndices{};
			std::array<uint32_t, kFrameContextCount> colliderSrvIndices{};
			std::array<bool, kFrameContextCount> hasReadback{};
			std::array<bool, kFrameContextCount> hasStateReadback{};
			uint32_t particleUavIndex{};
			uint32_t correctionUavIndex{};
			uint32_t bucketHeadUavIndex{};
			uint32_t particleCellUavIndex{};
			uint32_t particleNextUavIndex{};
			uint32_t counterUavIndex{};
			uint32_t retiredUavIndex{};
			std::vector<GpuPhysicsParticleCommand> commands;
			std::vector<CommandRangeForGPU> commandRanges;
			std::vector<GpuPhysicsCollider> colliders;
			std::vector<GpuRetiredPhysicsParticle> retiredParticles;
			std::vector<GpuPhysicsParticleState> latestStates;
			GpuPhysicsParticleStatistics latestStatistics{};
			uint32_t colliderCountPerSubstep{};
			float deltaTime{};
			bool needsInitialization = true;
			bool hasUpdate{};
			bool stateReadbackRequested{};
			std::vector<SphereDrawRequest> sphereDrawRequests;
			std::vector<ModelDrawRequest> modelDrawRequests;
		};

		struct WorldSlot {
			std::unique_ptr<World> world;
			uint32_t generation = 1u;
		};

	public:
		GpuPhysicsParticleBackend(
			DirectXCore& directX,
			PipelineManager& pipelineManager,
			Camera3DSystem& cameraSystem);
		~GpuPhysicsParticleBackend() override;

		bool Initialize() override;
		void BeginFrame() override;
		void EndFrame() override;
		void Finalize() override;

		GpuPhysicsParticleWorldHandle CreateWorld(const GpuPhysicsParticleWorldDesc& desc);
		bool RemoveWorld(GpuPhysicsParticleWorldHandle handle);
		bool ClearWorld(GpuPhysicsParticleWorldHandle handle);
		bool SetWorldSettings(
			GpuPhysicsParticleWorldHandle handle,
			const GpuPhysicsParticleWorldDesc& desc);
		bool UpdateWorld(
			GpuPhysicsParticleWorldHandle handle,
			float deltaTime,
			std::span<const GpuPhysicsParticleCommand> commands,
			std::span<const GpuPhysicsCollider> colliders,
			uint32_t colliderCountPerSubstep);
		GpuPhysicsParticleStatistics GetStatistics(GpuPhysicsParticleWorldHandle handle) const;
		bool ConsumeRetiredParticles(
			GpuPhysicsParticleWorldHandle handle,
			std::vector<GpuRetiredPhysicsParticle>& particles);
		bool RequestStates(GpuPhysicsParticleWorldHandle handle);
		bool GetLatestStates(
			GpuPhysicsParticleWorldHandle handle,
			std::vector<GpuPhysicsParticleState>& states,
			uint32_t& latencyFrames) const;
		bool GetLatestState(
			GpuPhysicsParticleWorldHandle handle,
			uint32_t particleIndex,
			GpuPhysicsParticleState& state,
			uint32_t& latencyFrames) const;
		bool DrawSpheres(
			GpuPhysicsParticleWorldHandle handle,
			uint32_t renderGroupIndex,
			const Vector4& color);
		void DrawQueued();
		bool DrawModel(
			GpuPhysicsParticleWorldHandle handle,
			uint32_t renderGroupIndex,
			const Model& model,
			const Vector3& scale,
			const Vector4& color);

	private:
		World* FindWorld(GpuPhysicsParticleWorldHandle handle);
		const World* FindWorld(GpuPhysicsParticleWorldHandle handle) const;
		bool InitializeWorld(World& world);
		void ReleaseWorldViews(World& world);
		void Readback(World& world, uint32_t frameIndex);
		void ReadbackStates(World& world, uint32_t frameIndex);
		void WriteFrameData(World& world, uint32_t frameIndex);
		void DispatchInitialize(World& world, uint32_t frameIndex);
		void DispatchSimulation(World& world, uint32_t frameIndex);
		void DispatchParticlePass(Pipeline& pipeline, World& world, uint32_t frameIndex, uint32_t substepIndex);
		void CopyReadback(World& world, uint32_t frameIndex);
		void CopyStateReadback(World& world, uint32_t frameIndex);
		void DispatchSphereDraws(World& world);
		void DispatchModelDraws(World& world);

	private:
		DirectXCore& directX_;
		PipelineManager& pipelineManager_;
		Camera3DSystem& cameraSystem_;
		Pipeline* initializePipeline_{};
		Pipeline* resetCountersPipeline_{};
		Pipeline* applyCommandsPipeline_{};
		Pipeline* integratePipeline_{};
		Pipeline* clearGridPipeline_{};
		Pipeline* buildGridPipeline_{};
		Pipeline* solvePositionsPipeline_{};
		Pipeline* applyPositionsPipeline_{};
		Pipeline* solveVelocitiesPipeline_{};
		Pipeline* applyVelocitiesPipeline_{};
		Pipeline* cleanupPipeline_{};
		Pipeline* sphereDrawPipeline_{};
		Pipeline* modelDrawPipeline_{};
		std::vector<WorldSlot> worlds_;
		std::vector<uint32_t> freeWorldIndices_;
	};

}
