#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Graphics/Drawer/3D/Base/BaseDrawer3D.h"
#include "LGF/Math/MathTypes.h"

namespace LGF {

	struct ModelResource;

	class ModelDrawer3D : public BaseDrawer3D {
	public:
		explicit ModelDrawer3D(Graphics3DContext context);
		~ModelDrawer3D() override;

		void Initialize() override;
		void EndFrame() override;

		void AddModel(
			std::shared_ptr<ModelResource> resource,
			const Matrix4x4& worldMatrix,
			const Matrix4x4& worldInverseTranspose,
			const Vector4& color,
			float worldMaximumScale,
			float worldOrientationSign,
			bool showMeshletColors,
			bool enableNormalMap,
			float normalScale,
			float metallicScale,
			float roughnessScale,
			float occlusionStrength,
			bool unlit
		);

	private:
		struct ModelInstanceData {
			Matrix4x4 worldMatrix;
			Matrix4x4 worldInverseTranspose;
			Vector4 color;
			float worldMaximumScale = 1.0f;
			uint32_t drawFlags = 0;
			float worldOrientationSign = 1.0f;
			float normalScale = 1.0f;
			float metallicScale = 1.0f;
			float roughnessScale = 1.0f;
			float occlusionStrength = 1.0f;
			float padding = 0.0f;
		};
		static_assert(sizeof(ModelInstanceData) == 176);

		struct ModelBatch {
			std::shared_ptr<ModelResource> resource;
			std::vector<ModelInstanceData> instances;
		};

		static constexpr uint32_t kMaxModelInstanceCount = 16384;
		static constexpr uint32_t kMaxDispatchCount = 8192;
		static constexpr uint32_t kConstantBufferStride = 256;

		ModelInstanceData* instanceData_ = nullptr;
		ComPtr<ID3D12Resource> drawConstantResource_;
		std::byte* drawConstantData_ = nullptr;
		std::vector<ModelBatch> batches_;
		std::unordered_map<const ModelResource*, size_t> batchIndices_;
		uint32_t queuedInstanceCount_ = 0;
	};

}
