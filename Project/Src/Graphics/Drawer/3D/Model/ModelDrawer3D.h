#pragma once

#include <cstddef>
#include <memory>
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
			bool showMeshletColors
		);

	private:
		struct DrawCommand {
			std::shared_ptr<ModelResource> resource;
			Matrix4x4 worldMatrix;
			Matrix4x4 worldInverseTranspose;
			Vector4 color;
			float worldMaximumScale = 1.0f;
			bool showMeshletColors = false;
		};

		static constexpr uint32_t kMaxMeshDrawCount = 4096;
		static constexpr uint32_t kConstantBufferStride = 256;

		ComPtr<ID3D12Resource> drawConstantResource_;
		std::byte* drawConstantData_ = nullptr;
		std::vector<DrawCommand> drawCommands_;
		uint32_t queuedMeshCount_ = 0;
	};

}
