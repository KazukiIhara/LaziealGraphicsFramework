#pragma once

#include "Graphics/DirectX/ComPtr/ComPtr.h"
#include "Graphics/DirectX/FrameContext/FrameContext.h"
#include "Graphics/Graphics3DContext.h"
#include "LGF/Graphics/GpuParticle3D.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace LGF {

	class Pipeline;

	// GPU上で生成・更新・破棄を行うParticle Worldの描画バックエンド。
	// 更新時にAliveListを構築し、描画時は生存粒子数分だけMesh Shaderを起動する。
	class GpuParticleDrawer3D {
	private:
		struct ParticleForGPU {
			Vector3 position{};
			float rotation{};
			Vector3 velocity{};
			float age{};
			Vector3 acceleration{};
			float lifetime{};
			Vector4 startColor{};
			Vector4 endColor{};
			Vector2 startSize{};
			Vector2 endSize{};
			float angularVelocity{};
			uint32_t textureIndex = UINT32_MAX;
			uint32_t alive{};
			float padding{};
		};
		static_assert(sizeof(ParticleForGPU) == 112u);

		struct SpawnBatchForGPU {
			Vector3 position{};
			float minimumLifetime{};
			Vector3 minimumVelocity{};
			float maximumLifetime{};
			Vector3 maximumVelocity{};
			float minimumRotation{};
			Vector3 acceleration{};
			float maximumRotation{};
			Vector4 startColor{};
			Vector4 endColor{};
			Vector2 startSize{};
			Vector2 endSize{};
			float minimumAngularVelocity{};
			float maximumAngularVelocity{};
			uint32_t count{};
			uint32_t randomSeed{};
			uint32_t textureIndex = UINT32_MAX;
			uint32_t padding[3]{};
		};
		static_assert(sizeof(SpawnBatchForGPU) == 144u);

		struct ConstantsForGPU {
			Vector3 worldMinimum{};
			float deltaTime{};
			Vector3 worldMaximum{};
			float boundsPadding{};
			uint32_t maximumParticleCount{};
			uint32_t spawnBatchCount{};
			uint32_t removeOutOfBounds{};
			uint32_t padding{};
		};
		static_assert(sizeof(ConstantsForGPU) == 48u);

		struct CountersForGPU {
			uint32_t freeCount{};
			uint32_t aliveCount{};
			uint32_t generatedCount{};
			uint32_t rejectedCount{};
			uint32_t expiredCount{};
			uint32_t outOfBoundsCount{};
		};
		static_assert(sizeof(CountersForGPU) == 24u);

		// ExecuteIndirectがDispatchMeshの引数として読むGPU側のコマンドデータ。
		struct DrawArgumentsForGPU {
			uint32_t threadGroupCountX{};
			uint32_t threadGroupCountY = 1u;
			uint32_t threadGroupCountZ = 1u;
		};
		static_assert(sizeof(DrawArgumentsForGPU) == sizeof(D3D12_DISPATCH_MESH_ARGUMENTS));

		struct World {
			GpuParticleWorldDesc desc{};
			ComPtr<ID3D12Resource> particleResource;
			ComPtr<ID3D12Resource> freeListResource;
			ComPtr<ID3D12Resource> counterResource;
			ComPtr<ID3D12Resource> aliveListResource;
			ComPtr<ID3D12Resource> aliveCountResource;
			ComPtr<ID3D12Resource> drawArgumentResource;
			FrameBufferedUploadBuffer spawnResources;
			FrameBufferedUploadBuffer constantResources;
			std::array<ComPtr<ID3D12Resource>, kFrameContextCount> readbackResources{};
			std::array<uint32_t, kFrameContextCount> spawnSrvIndices{};
			std::array<bool, kFrameContextCount> hasReadback{};
			uint32_t particleUavIndex{};
			uint32_t freeListUavIndex{};
			uint32_t counterUavIndex{};
			uint32_t aliveListUavIndex{};
			uint32_t aliveCountUavIndex{};
			uint32_t drawArgumentUavIndex{};
			std::vector<GpuParticleSpawnBatch3D> spawnBatches;
			GpuParticleStatistics3D latestStatistics{};
			bool needsInitialization = true;
			bool drawArgumentsInitialized{};
			bool hasUpdate{};
			bool shouldDraw{};
			float deltaTime{};
		};

		struct WorldSlot {
			std::unique_ptr<World> world;
			uint32_t generation = 1u;
		};

	public:
		explicit GpuParticleDrawer3D(Graphics3DContext context);
		~GpuParticleDrawer3D();

		bool Initialize();
		void BeginFrame();
		void EndFrame();
		void Finalize();

		GpuParticleWorldHandle CreateWorld(const GpuParticleWorldDesc& desc);
		bool RemoveWorld(GpuParticleWorldHandle handle);
		bool ClearWorld(GpuParticleWorldHandle handle);
		bool UpdateWorld(GpuParticleWorldHandle handle, float deltaTime, std::span<const GpuParticleSpawnBatch3D> spawnBatches);
		bool DrawWorld(GpuParticleWorldHandle handle);
		GpuParticleStatistics3D GetStatistics(GpuParticleWorldHandle handle) const;

	private:
		World* FindWorld(GpuParticleWorldHandle handle);
		const World* FindWorld(GpuParticleWorldHandle handle) const;
		bool InitializeWorld(World& world);
		void ReadStatistics(World& world, uint32_t frameIndex);
		void WriteFrameData(World& world, uint32_t frameIndex);
		void DispatchInitialize(World& world);
		void DispatchSimulation(World& world, uint32_t frameIndex);
		void DispatchBuildDrawArguments(World& world);
		void DispatchDraw(World& world);
		void CopyStatistics(World& world, uint32_t frameIndex);

	private:
		Graphics3DContext context_;
		Pipeline* initializePipeline_{};
		Pipeline* resetStatisticsPipeline_{};
		Pipeline* spawnPipeline_{};
		Pipeline* updatePipeline_{};
		Pipeline* buildDrawArgumentsPipeline_{};
		Pipeline* drawPipeline_{};
		ComPtr<ID3D12CommandSignature> drawCommandSignature_;
		std::vector<WorldSlot> worlds_;
		std::vector<uint32_t> freeWorldIndices_;
	};

}
