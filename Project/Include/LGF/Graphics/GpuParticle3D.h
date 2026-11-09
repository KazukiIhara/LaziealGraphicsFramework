#pragma once

#include "LGF/Graphics/Texture.h"
#include "LGF/Math/MathTypes.h"

#include <cstdint>
#include <limits>

namespace LGF {

	// Renderer内部のGPU Particle Worldを世代付きで参照する軽量ハンドル。
	struct GpuParticleWorldHandle {
		static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

		uint32_t index = InvalidIndex;
		uint32_t generation{};

		bool IsValid() const { return index != InvalidIndex && generation != 0u; }
		bool operator==(const GpuParticleWorldHandle&) const = default;
	};

	// GPUリソースは生成後に拡張しない。上限を変える場合はWorldを作り直す。
	struct GpuParticleWorldDesc {
		Vector3 worldMinimum{ -100.0f, -100.0f, -100.0f };
		Vector3 worldMaximum{ 100.0f, 100.0f, 100.0f };
		uint32_t maximumParticleCount = 10000u;
		uint32_t maximumSpawnBatchCount = 256u;
		float boundsPadding{};
		bool removeOutOfBounds = true;
	};

	// 1つのEmitterから同じフレームに生成する粒子群。
	// 粒子ごとの初期値はCompute Shader側でseedから生成する。
	struct GpuParticleSpawnBatch3D {
		Vector3 position{};
		Vector3 minimumVelocity{};
		Vector3 maximumVelocity{};
		Vector3 acceleration{};
		float minimumLifetime = 1.0f;
		float maximumLifetime = 1.0f;
		Vector4 startColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		Vector4 endColor{ 1.0f, 1.0f, 1.0f, 0.0f };
		Vector2 startSize{ 0.1f, 0.1f };
		Vector2 endSize{};
		float minimumRotation{};
		float maximumRotation{};
		float minimumAngularVelocity{};
		float maximumAngularVelocity{};
		uint32_t count{};
		uint32_t randomSeed{};
		Texture texture{};
	};

	// GPUから非同期で回収した統計。通常は2フレーム前の完了値を返す。
	struct GpuParticleStatistics3D {
		uint32_t particleCount{};
		uint32_t generatedCount{};
		uint32_t rejectedCount{};
		uint32_t expiredCount{};
		uint32_t outOfBoundsCount{};
		uint32_t latencyFrames{};
		bool isAvailable{};
	};

}
