#pragma once

#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"
#include "Graphics/DirectX/FrameContext/FrameContext.h"
#include "LGF/Math/MathTypes.h"

namespace LGF {

	struct ModelVertex {
		Vector3 position;
		Vector3 normal;
		Vector2 uv;
		Vector4 tangent;
	};

	struct ModelVertexInfluence {
		uint32_t boneIndices[4]{};
		float weights[4]{};
	};

	struct ModelNodeTransform {
		Vector3 scale{ 1.0f, 1.0f, 1.0f };
		Quaternion rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		Vector3 translation{};
	};

	template<class T>
	struct ModelAnimationKeyframe {
		float time = 0.0f;
		T value{};
	};

	struct ModelAnimationTrack {
		uint32_t nodeIndex = 0;
		std::vector<ModelAnimationKeyframe<Vector3>> scales;
		std::vector<ModelAnimationKeyframe<Quaternion>> rotations;
		std::vector<ModelAnimationKeyframe<Vector3>> translations;
	};

	struct ModelAnimationClip {
		std::string name;
		float duration = 0.0f;
		std::vector<ModelAnimationTrack> tracks;
	};

	struct ModelNodeResource {
		std::string name;
		int32_t parentIndex = -1;
		ModelNodeTransform bindTransform;
	};

	struct ModelBoneResource {
		uint32_t nodeIndex = 0;
		Matrix4x4 inverseBindMatrix;
	};

	struct ModelRigResource {
		std::vector<ModelNodeResource> nodes;
		std::vector<ModelBoneResource> bones;
		std::vector<ModelAnimationClip> animations;
		std::unordered_map<std::string, size_t> animationIndices;
		Matrix4x4 inverseRootMatrix;
	};
	static_assert(sizeof(ModelVertex) == 48);

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
		ComPtr<ID3D12Resource> influenceResource;
		uint32_t vertexSrvIndex = UINT32_MAX;
		uint32_t meshletSrvIndex = UINT32_MAX;
		uint32_t meshletVertexIndexSrvIndex = UINT32_MAX;
		uint32_t meshletPrimitiveSrvIndex = UINT32_MAX;
		uint32_t meshletCullDataSrvIndex = UINT32_MAX;
		uint32_t influenceSrvIndex = UINT32_MAX;
		uint32_t vertexCount = 0;
		uint32_t meshletCount = 0;
		uint32_t meshletIndexOffset = 0;
		uint32_t baseColorTextureSrvIndex = UINT32_MAX;
		uint32_t normalTextureSrvIndex = UINT32_MAX;
		uint32_t metallicRoughnessTextureSrvIndex = UINT32_MAX;
		uint32_t occlusionTextureSrvIndex = UINT32_MAX;
		float normalScale = 1.0f;
		float metallicFactor = 0.0f;
		float roughnessFactor = 1.0f;
		float occlusionStrength = 1.0f;
		Vector4 baseColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		std::vector<ModelMeshletBounds> meshletBounds;
	};

	struct ModelResource {
		std::filesystem::path filePath;
		std::vector<ModelMeshResource> meshes;
		std::optional<ModelRigResource> rig;
	};

	struct ModelSkinningPaletteEntry {
		Matrix4x4 positionMatrix;
		Matrix4x4 normalMatrix;
	};

	struct ModelSkinnedMeshResource {
		ComPtr<ID3D12Resource> vertexResource;
		uint32_t vertexSrvIndex = UINT32_MAX;
		uint32_t vertexUavIndex = UINT32_MAX;
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
	};

	struct ModelAnimatorResource {
		std::shared_ptr<ModelResource> model;
		std::vector<ModelNodeTransform> localTransforms;
		std::vector<ModelNodeTransform> fadeSourceTransforms;
		std::vector<Matrix4x4> globalMatrices;
		std::vector<ModelSkinningPaletteEntry> palette;
		std::vector<ModelSkinnedMeshResource> meshes;
		FrameBufferedUploadBuffer paletteResources;
		std::array<uint32_t, kFrameContextCount> paletteSrvIndices{};
		bool gpuResourcesInitialized = false;
		size_t animationIndex = SIZE_MAX;
		float time = 0.0f;
		float playbackSpeed = 1.0f;
		float fadeTime = 0.0f;
		float fadeDuration = 0.0f;
		bool loop = true;
		bool playing = false;
	};

}
