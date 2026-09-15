#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"
#include "LGF/Math/MathTypes.h"

namespace LGF {

	struct ModelVertex {
		Vector3 position;
		Vector3 normal;
		Vector2 uv;
	};
	static_assert(sizeof(ModelVertex) == 32);

	struct ModelMeshletBounds {
		Vector3 center;
		float radius = 0.0f;
	};
	static_assert(sizeof(ModelMeshletBounds) == 16);

	struct ModelMeshResource {
		ComPtr<ID3D12Resource> vertexResource;
		ComPtr<ID3D12Resource> meshletResource;
		ComPtr<ID3D12Resource> meshletVertexIndexResource;
		ComPtr<ID3D12Resource> meshletPrimitiveResource;
		ComPtr<ID3D12Resource> meshletCullDataResource;
		uint32_t vertexSrvIndex = UINT32_MAX;
		uint32_t meshletSrvIndex = UINT32_MAX;
		uint32_t meshletVertexIndexSrvIndex = UINT32_MAX;
		uint32_t meshletPrimitiveSrvIndex = UINT32_MAX;
		uint32_t meshletCullDataSrvIndex = UINT32_MAX;
		uint32_t meshletCount = 0;
		uint32_t meshletIndexOffset = 0;
		uint32_t baseColorTextureSrvIndex = UINT32_MAX;
		Vector4 baseColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		std::vector<ModelMeshletBounds> meshletBounds;
	};

	struct ModelResource {
		std::string filePath;
		std::vector<ModelMeshResource> meshes;
	};

}
