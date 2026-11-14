#pragma once

namespace LGF {
	class GpuPhysicsParticleBackend;

	namespace GpuCompute::Internal {
		void SetPhysicsParticleBackend(GpuPhysicsParticleBackend* backend);
		void ResetPhysicsParticleBackend();
	}
}
