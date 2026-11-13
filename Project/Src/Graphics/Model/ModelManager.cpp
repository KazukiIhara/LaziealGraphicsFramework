#include "ModelManager.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <format>
#include <functional>
#include <cmath>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

#include <d3d12.h>
#include <DirectXMesh.h>

#include <assimp/Importer.hpp>
#include <assimp/GltfMaterial.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Texture/TextureManager.h"
#include "LGF/Graphics/Texture.h"
#include "LGF/Math/MathUtils.h"
#include "ModelResource.h"

using namespace LGF;

namespace {

	std::filesystem::path ToPath(const std::string& utf8Path) {
		return std::filesystem::path(std::u8string(utf8Path.begin(), utf8Path.end()));
	}

	std::string ToUtf8String(const std::filesystem::path& path) {
		const auto value = path.u8string();
		return std::string(value.begin(), value.end());
	}

	template<class T>
	ComPtr<ID3D12Resource> CreateUploadBuffer(
		DirectXCore& directX,
		const std::vector<T>& source) {

		if (source.empty()) {
			throw std::runtime_error("Model buffer source must not be empty.");
		}

		auto resource = directX.GetResourceFactory()->CreateUploadBufferResource(
			source.size() * sizeof(T));
		if (!resource) {
			throw std::runtime_error("Model buffer allocation failed.");
		}

		void* mapped = nullptr;
		const HRESULT hr = resource->Map(0, nullptr, &mapped);
		if (FAILED(hr) || !mapped) {
			throw std::runtime_error(std::format(
				"Model buffer mapping failed (0x{:08X}).",
				static_cast<uint32_t>(hr)));
		}
		std::memcpy(mapped, source.data(), source.size() * sizeof(T));
		resource->Unmap(0, nullptr);
		return resource;
	}

	const aiTextureType kBaseColorTextureTypes[] = {
		aiTextureType_BASE_COLOR,
		aiTextureType_DIFFUSE,
	};

	const aiTextureType kMetallicRoughnessTextureTypes[] = {
		aiTextureType_UNKNOWN,
		aiTextureType_METALNESS,
		aiTextureType_DIFFUSE_ROUGHNESS,
	};

	const aiTextureType kOcclusionTextureTypes[] = {
		aiTextureType_AMBIENT_OCCLUSION,
		aiTextureType_LIGHTMAP,
	};

	uint32_t LoadMaterialTexture(
		TextureManager& textureManager,
		const aiScene& scene,
		const std::filesystem::path& modelPath,
		const aiString& textureReference,
		const TextureLoadDesc& loadDesc) {

		const std::string reference(textureReference.C_Str(), textureReference.length);
		const aiTexture* embeddedTexture = scene.GetEmbeddedTexture(textureReference.C_Str());
		if (embeddedTexture) {
			if (embeddedTexture->mHeight != 0) {
				throw std::runtime_error(std::format(
					"Model '{}': uncompressed embedded textures are not supported yet.",
					ToUtf8String(modelPath)));
			}
			const std::wstring textureKey = std::format(
				L"{}|embedded:{}",
				modelPath.native(),
				std::wstring(reference.begin(), reference.end()));
			return textureManager.LoadFromMemory(
				textureKey,
				embeddedTexture->pcData,
				embeddedTexture->mWidth,
				loadDesc).srvIndex;
		}

		const auto texturePath =
			(modelPath.parent_path() / ToPath(reference)).lexically_normal();
		return textureManager.Load(texturePath, loadDesc).srvIndex;
	}

	constexpr size_t kMaxMeshletVertices = 128;
	constexpr size_t kMaxMeshletPrimitives = 128;
	static_assert(sizeof(DirectX::Meshlet) == 16);
	static_assert(sizeof(DirectX::MeshletTriangle) == sizeof(uint32_t));
	static_assert(sizeof(DirectX::CullData) == 24);

	Matrix4x4 ConvertMatrix(const aiMatrix4x4& matrix) {
		return { {
			{ matrix.a1, matrix.b1, matrix.c1, matrix.d1 },
			{ matrix.a2, matrix.b2, matrix.c2, matrix.d2 },
			{ matrix.a3, matrix.b3, matrix.c3, matrix.d3 },
			{ matrix.a4, matrix.b4, matrix.c4, matrix.d4 },
		} };
	}

	ModelNodeTransform ConvertTransform(const aiMatrix4x4& matrix) {
		aiVector3D scale;
		aiQuaternion rotation;
		aiVector3D translation;
		matrix.Decompose(scale, rotation, translation);
		return {
			.scale = { scale.x, scale.y, scale.z },
			.rotation = { rotation.x, rotation.y, rotation.z, rotation.w },
			.translation = { translation.x, translation.y, translation.z },
		};
	}

	bool NearlyEqual(const Matrix4x4& a, const Matrix4x4& b) {
		for (size_t row = 0; row < 4; ++row) {
			for (size_t column = 0; column < 4; ++column) {
				if (std::abs(a.m[row][column] - b.m[row][column]) > 1.0e-4f) {
					return false;
				}
			}
		}
		return true;
	}

}

ModelManager::ModelManager(DirectXCore& directX, TextureManager& textureManager) :
	directX_(directX),
	textureManager_(textureManager) {
}

ModelManager::~ModelManager() = default;

bool ModelManager::Initialize() {
	return true;
}

void ModelManager::Finalize() {
	models_.clear();
}

std::shared_ptr<ModelResource> ModelManager::Load(const std::filesystem::path& filePath) {
	if (filePath.empty()) {
		throw std::invalid_argument("Model path must not be empty.");
	}

	const auto path = std::filesystem::absolute(filePath).lexically_normal();
	const std::string debugName = ToUtf8String(filePath);
	if (const auto it = models_.find(path.native()); it != models_.end()) {
		return it->second;
	}

	Assimp::Importer importer;
	constexpr unsigned int kImportFlags =
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_ImproveCacheLocality |
		aiProcess_ConvertToLeftHanded |
		aiProcess_ValidateDataStructure;
	const aiScene* scene = importer.ReadFile(ToUtf8String(path), kImportFlags);
	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0 || !scene->mRootNode) {
		throw std::runtime_error(std::format(
			"Model '{}': Assimp import failed: {}",
			debugName,
			importer.GetErrorString()));
	}

	bool hasRig = false;
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		hasRig |= scene->mMeshes[meshIndex] && scene->mMeshes[meshIndex]->HasBones();
	}
	if (!hasRig) {
		scene = importer.ApplyPostProcessing(aiProcess_PreTransformVertices);
		if (!scene) {
			throw std::runtime_error(std::format(
				"Model '{}': static vertex pre-transform failed: {}",
				debugName,
				importer.GetErrorString()));
		}
	}

	auto model = std::make_shared<ModelResource>();
	model->filePath = filePath;
	model->meshes.reserve(scene->mNumMeshes);
	uint32_t meshletIndexOffset = 0;
	std::unordered_map<std::string, uint32_t> nodeIndices;
	std::unordered_map<std::string, uint32_t> boneIndices;
	if (hasRig) {
		model->rig.emplace();
		std::function<void(const aiNode*, int32_t)> loadNode =
			[&](const aiNode* sourceNode, int32_t parentIndex) {
				const std::string name = sourceNode->mName.C_Str();
				if (nodeIndices.contains(name)) {
					throw std::runtime_error(std::format(
						"Model '{}': animated node name '{}' is duplicated.", debugName, name));
				}
				const uint32_t nodeIndex = static_cast<uint32_t>(model->rig->nodes.size());
				nodeIndices.emplace(name, nodeIndex);
				model->rig->nodes.push_back({
					.name = name,
					.parentIndex = parentIndex,
					.bindTransform = ConvertTransform(sourceNode->mTransformation),
				});
				for (uint32_t childIndex = 0; childIndex < sourceNode->mNumChildren; ++childIndex) {
					loadNode(sourceNode->mChildren[childIndex], static_cast<int32_t>(nodeIndex));
				}
			};
		loadNode(scene->mRootNode, -1);
		model->rig->inverseRootMatrix = Math::Inverse(ConvertMatrix(scene->mRootNode->mTransformation));
	}

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		const aiMesh* sourceMesh = scene->mMeshes[meshIndex];
		if (!sourceMesh || sourceMesh->mNumVertices == 0 || sourceMesh->mNumFaces == 0) {
			continue;
		}
		if (!sourceMesh->HasNormals()) {
			throw std::runtime_error(std::format(
				"Model '{}': mesh {} has no normals.",
				debugName,
				meshIndex));
		}

		std::vector<ModelVertex> vertices(sourceMesh->mNumVertices);
		std::vector<ModelVertexInfluence> influences(sourceMesh->mNumVertices);
		for (uint32_t vertexIndex = 0; vertexIndex < sourceMesh->mNumVertices; ++vertexIndex) {
			const aiVector3D& position = sourceMesh->mVertices[vertexIndex];
			const aiVector3D& normal = sourceMesh->mNormals[vertexIndex];
			const aiVector3D uv = sourceMesh->HasTextureCoords(0)
				? sourceMesh->mTextureCoords[0][vertexIndex]
				: aiVector3D{};

			Vector4 tangent{};
			if (sourceMesh->HasTangentsAndBitangents()) {
				const aiVector3D& sourceTangent = sourceMesh->mTangents[vertexIndex];
				const aiVector3D& sourceBitangent = sourceMesh->mBitangents[vertexIndex];
				const aiVector3D normalCrossTangent{
					normal.y * sourceTangent.z - normal.z * sourceTangent.y,
					normal.z * sourceTangent.x - normal.x * sourceTangent.z,
					normal.x * sourceTangent.y - normal.y * sourceTangent.x,
				};
				const float handedness =
					normalCrossTangent.x * sourceBitangent.x +
					normalCrossTangent.y * sourceBitangent.y +
					normalCrossTangent.z * sourceBitangent.z < 0.0f
					? -1.0f
					: 1.0f;
				tangent = {
					sourceTangent.x,
					sourceTangent.y,
					sourceTangent.z,
					handedness,
				};
			}
			vertices[vertexIndex] = {
				.position = { position.x, position.y, position.z },
				.normal = { normal.x, normal.y, normal.z },
				.uv = { uv.x, uv.y },
				.tangent = tangent,
			};
		}

		if (hasRig) {
			std::vector<std::vector<std::pair<uint32_t, float>>> vertexWeights(sourceMesh->mNumVertices);
			for (uint32_t sourceBoneIndex = 0; sourceBoneIndex < sourceMesh->mNumBones; ++sourceBoneIndex) {
				const aiBone* sourceBone = sourceMesh->mBones[sourceBoneIndex];
				const std::string boneName = sourceBone->mName.C_Str();
				const auto nodeIt = nodeIndices.find(boneName);
				if (nodeIt == nodeIndices.end()) {
					throw std::runtime_error(std::format(
						"Model '{}': bone '{}' has no matching node.", debugName, boneName));
				}
				uint32_t boneIndex = 0;
				if (const auto it = boneIndices.find(boneName); it != boneIndices.end()) {
					boneIndex = it->second;
					if (!NearlyEqual(
						model->rig->bones[boneIndex].inverseBindMatrix,
						ConvertMatrix(sourceBone->mOffsetMatrix))) {
						throw std::runtime_error(std::format(
							"Model '{}': bone '{}' uses different inverse bind matrices across meshes.",
							debugName,
							boneName));
					}
				} else {
					boneIndex = static_cast<uint32_t>(model->rig->bones.size());
					boneIndices.emplace(boneName, boneIndex);
					model->rig->bones.push_back({
						.nodeIndex = nodeIt->second,
						.inverseBindMatrix = ConvertMatrix(sourceBone->mOffsetMatrix),
					});
				}
				for (uint32_t weightIndex = 0; weightIndex < sourceBone->mNumWeights; ++weightIndex) {
					const aiVertexWeight& weight = sourceBone->mWeights[weightIndex];
					if (weight.mVertexId < vertexWeights.size() && weight.mWeight > 0.0f) {
						vertexWeights[weight.mVertexId].emplace_back(boneIndex + 1u, weight.mWeight);
					}
				}
			}
			for (uint32_t vertexIndex = 0; vertexIndex < sourceMesh->mNumVertices; ++vertexIndex) {
				auto& weights = vertexWeights[vertexIndex];
				std::ranges::sort(
					weights,
					std::ranges::less{},
					&std::pair<uint32_t, float>::second);
				std::ranges::reverse(weights);
				float totalWeight = 0.0f;
				const size_t influenceCount = std::min<size_t>(weights.size(), 4);
				for (size_t influenceIndex = 0; influenceIndex < influenceCount; ++influenceIndex) {
					influences[vertexIndex].boneIndices[influenceIndex] = weights[influenceIndex].first;
					influences[vertexIndex].weights[influenceIndex] = weights[influenceIndex].second;
					totalWeight += weights[influenceIndex].second;
				}
				if (totalWeight > 0.0f) {
					for (float& weight : influences[vertexIndex].weights) {
						weight /= totalWeight;
					}
				} else {
					influences[vertexIndex].weights[0] = 1.0f;
				}
			}
		}

		std::vector<uint32_t> indices;
		indices.reserve(static_cast<size_t>(sourceMesh->mNumFaces) * 3);
		for (uint32_t faceIndex = 0; faceIndex < sourceMesh->mNumFaces; ++faceIndex) {
			const aiFace& face = sourceMesh->mFaces[faceIndex];
			if (face.mNumIndices != 3) {
				throw std::runtime_error(std::format(
					"Model '{}': mesh {} contains a non-triangle face after triangulation.",
					debugName,
					meshIndex));
			}
			indices.insert(indices.end(), face.mIndices, face.mIndices + 3);
		}

		std::vector<DirectX::XMFLOAT3> positions;
		positions.reserve(vertices.size());
		for (const ModelVertex& vertex : vertices) {
			positions.emplace_back(vertex.position.x, vertex.position.y, vertex.position.z);
		}

		std::vector<DirectX::Meshlet> meshlets;
		std::vector<uint8_t> meshletVertexIndexBytes;
		std::vector<DirectX::MeshletTriangle> meshletPrimitives;
		const HRESULT meshletHr = DirectX::ComputeMeshlets(
			indices.data(),
			indices.size() / 3,
			positions.data(),
			positions.size(),
			nullptr,
			meshlets,
			meshletVertexIndexBytes,
			meshletPrimitives,
			kMaxMeshletVertices,
			kMaxMeshletPrimitives);
		if (FAILED(meshletHr)) {
			throw std::runtime_error(std::format(
				"Model '{}': meshlet generation failed for mesh {} (0x{:08X}).",
				debugName,
				meshIndex,
				static_cast<uint32_t>(meshletHr)));
		}
		if (meshlets.empty() || meshletVertexIndexBytes.empty() || meshletPrimitives.empty()) {
			throw std::runtime_error(std::format(
				"Model '{}': meshlet generation produced no drawable data for mesh {}.",
				debugName,
				meshIndex));
		}
		if ((meshletVertexIndexBytes.size() % sizeof(uint32_t)) != 0) {
			throw std::runtime_error("DirectXMesh returned an invalid 32-bit vertex index buffer.");
		}

		std::vector<uint32_t> meshletVertexIndices(
			meshletVertexIndexBytes.size() / sizeof(uint32_t));
		std::memcpy(
			meshletVertexIndices.data(),
			meshletVertexIndexBytes.data(),
			meshletVertexIndexBytes.size());

		std::vector<DirectX::CullData> meshletCullData(meshlets.size());
		const HRESULT cullDataHr = DirectX::ComputeCullData(
			positions.data(),
			positions.size(),
			meshlets.data(),
			meshlets.size(),
			meshletVertexIndices.data(),
			meshletVertexIndices.size(),
			meshletPrimitives.data(),
			meshletPrimitives.size(),
			meshletCullData.data(),
			DirectX::MESHLET_WIND_CW);
		if (FAILED(cullDataHr)) {
			throw std::runtime_error(std::format(
				"Model '{}': meshlet cull-data generation failed for mesh {} (0x{:08X}).",
				debugName,
				meshIndex,
				static_cast<uint32_t>(cullDataHr)));
		}

		ModelMeshResource mesh{};
		mesh.vertexResource = CreateUploadBuffer(directX_, vertices);
		mesh.meshletResource = CreateUploadBuffer(directX_, meshlets);
		mesh.meshletVertexIndexResource = CreateUploadBuffer(directX_, meshletVertexIndices);
		mesh.meshletPrimitiveResource = CreateUploadBuffer(directX_, meshletPrimitives);
		mesh.meshletCullDataResource = CreateUploadBuffer(directX_, meshletCullData);
		mesh.vertexCount = static_cast<uint32_t>(vertices.size());
		if (hasRig) {
			mesh.influenceResource = CreateUploadBuffer(directX_, influences);
		}
		mesh.meshletCount = static_cast<uint32_t>(meshlets.size());
		mesh.meshletIndexOffset = meshletIndexOffset;
		meshletIndexOffset += mesh.meshletCount;
		mesh.meshletBounds.reserve(meshletCullData.size());
		for (const DirectX::CullData& cullData : meshletCullData) {
			mesh.meshletBounds.push_back({
				.center = {
					cullData.BoundingSphere.Center.x,
					cullData.BoundingSphere.Center.y,
					cullData.BoundingSphere.Center.z },
				.radius = cullData.BoundingSphere.Radius,
			});
		}

		auto* views = directX_.GetResMgr().srvuav;
		if (!views->IsLowerViewMax()) {
			throw std::runtime_error("Model descriptor heap is full.");
		}
		mesh.vertexSrvIndex = views->Allocate();
		views->CreateSrvStructuredBuffer(
			mesh.vertexSrvIndex,
			mesh.vertexResource.Get(),
			static_cast<uint32_t>(vertices.size()),
			static_cast<UINT>(sizeof(ModelVertex)));
		if (hasRig) {
			if (!views->IsLowerViewMax()) {
				throw std::runtime_error("Model descriptor heap is full.");
			}
			mesh.influenceSrvIndex = views->Allocate();
			views->CreateSrvStructuredBuffer(
				mesh.influenceSrvIndex,
				mesh.influenceResource.Get(),
				static_cast<uint32_t>(influences.size()),
				static_cast<UINT>(sizeof(ModelVertexInfluence)));
		}

		if (!views->IsLowerViewMax()) {
			throw std::runtime_error("Model descriptor heap is full.");
		}
		mesh.meshletSrvIndex = views->Allocate();
		views->CreateSrvStructuredBuffer(
			mesh.meshletSrvIndex,
			mesh.meshletResource.Get(),
			static_cast<uint32_t>(meshlets.size()),
			static_cast<UINT>(sizeof(DirectX::Meshlet)));

		if (!views->IsLowerViewMax()) {
			throw std::runtime_error("Model descriptor heap is full.");
		}
		mesh.meshletVertexIndexSrvIndex = views->Allocate();
		views->CreateSrvStructuredBuffer(
			mesh.meshletVertexIndexSrvIndex,
			mesh.meshletVertexIndexResource.Get(),
			static_cast<uint32_t>(meshletVertexIndices.size()),
			static_cast<UINT>(sizeof(uint32_t)));

		if (!views->IsLowerViewMax()) {
			throw std::runtime_error("Model descriptor heap is full.");
		}
		mesh.meshletPrimitiveSrvIndex = views->Allocate();
		views->CreateSrvStructuredBuffer(
			mesh.meshletPrimitiveSrvIndex,
			mesh.meshletPrimitiveResource.Get(),
			static_cast<uint32_t>(meshletPrimitives.size()),
			static_cast<UINT>(sizeof(DirectX::MeshletTriangle)));

		if (!views->IsLowerViewMax()) {
			throw std::runtime_error("Model descriptor heap is full.");
		}
		mesh.meshletCullDataSrvIndex = views->Allocate();
		views->CreateSrvStructuredBuffer(
			mesh.meshletCullDataSrvIndex,
			mesh.meshletCullDataResource.Get(),
			static_cast<uint32_t>(meshletCullData.size()),
			static_cast<UINT>(sizeof(DirectX::CullData)));

		if (sourceMesh->mMaterialIndex < scene->mNumMaterials) {
			const aiMaterial* material = scene->mMaterials[sourceMesh->mMaterialIndex];
			aiColor4D baseColor{ 1.0f, 1.0f, 1.0f, 1.0f };
			if (material->Get(AI_MATKEY_BASE_COLOR, baseColor) != AI_SUCCESS) {
				material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
			}
			mesh.baseColor = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };

			aiString textureReference;
			bool hasBaseColorTexture = false;
			for (const aiTextureType type : kBaseColorTextureTypes) {
				if (material->GetTexture(type, 0, &textureReference) == AI_SUCCESS) {
					hasBaseColorTexture = true;
					break;
				}
			}

			if (hasBaseColorTexture) {
				mesh.baseColorTextureSrvIndex = LoadMaterialTexture(
					textureManager_,
					*scene,
					path,
					textureReference,
					TextureLoadDesc::Color());
			}

			aiString normalTextureReference;
			if (material->GetTexture(
				aiTextureType_NORMALS,
				0,
				&normalTextureReference) == AI_SUCCESS) {
				mesh.normalTextureSrvIndex = LoadMaterialTexture(
					textureManager_,
					*scene,
					path,
					normalTextureReference,
					TextureLoadDesc::NormalMap());

				float normalScale = 1.0f;
				if (material->Get(
					AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0),
					normalScale) == AI_SUCCESS) {
					mesh.normalScale = normalScale;
				}
			}

			// PBR情報を持たない旧形式モデルは、従来の見た目に近い非金属として扱う。
			float metallicFactor = 0.0f;
			material->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor);
			mesh.metallicFactor = std::clamp(metallicFactor, 0.0f, 1.0f);

			float roughnessFactor = 1.0f;
			material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor);
			mesh.roughnessFactor = std::clamp(roughnessFactor, 0.0f, 1.0f);

			aiString metallicRoughnessTextureReference;
			for (const aiTextureType type : kMetallicRoughnessTextureTypes) {
				if (material->GetTexture(
					type,
					0,
					&metallicRoughnessTextureReference) == AI_SUCCESS) {
					mesh.metallicRoughnessTextureSrvIndex = LoadMaterialTexture(
						textureManager_,
						*scene,
						path,
						metallicRoughnessTextureReference,
						TextureLoadDesc::Data());
					break;
				}
			}

			aiString occlusionTextureReference;
			aiTextureType occlusionTextureType = aiTextureType_NONE;
			for (const aiTextureType type : kOcclusionTextureTypes) {
				if (material->GetTexture(type, 0, &occlusionTextureReference) == AI_SUCCESS) {
					occlusionTextureType = type;
					mesh.occlusionTextureSrvIndex = LoadMaterialTexture(
						textureManager_,
						*scene,
						path,
						occlusionTextureReference,
						TextureLoadDesc::Data());
					break;
				}
			}

			if (occlusionTextureType != aiTextureType_NONE) {
				float occlusionStrength = 1.0f;
				if (material->Get(
					AI_MATKEY_GLTF_TEXTURE_STRENGTH(occlusionTextureType, 0),
					occlusionStrength) == AI_SUCCESS) {
					mesh.occlusionStrength = std::clamp(occlusionStrength, 0.0f, 1.0f);
				}
			}
		}

		model->meshes.push_back(std::move(mesh));
	}

	if (hasRig) {
		for (uint32_t animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex) {
			const aiAnimation* sourceAnimation = scene->mAnimations[animationIndex];
			const double ticksPerSecond = sourceAnimation->mTicksPerSecond > 0.0
				? sourceAnimation->mTicksPerSecond
				: 25.0;
			ModelAnimationClip clip{};
			clip.name = sourceAnimation->mName.length > 0
				? sourceAnimation->mName.C_Str()
				: std::format("Animation{}", animationIndex);
			clip.duration = static_cast<float>(sourceAnimation->mDuration / ticksPerSecond);
			clip.tracks.reserve(sourceAnimation->mNumChannels);
			for (uint32_t channelIndex = 0; channelIndex < sourceAnimation->mNumChannels; ++channelIndex) {
				const aiNodeAnim* sourceTrack = sourceAnimation->mChannels[channelIndex];
				const auto nodeIt = nodeIndices.find(sourceTrack->mNodeName.C_Str());
				if (nodeIt == nodeIndices.end()) {
					continue;
				}
				ModelAnimationTrack track{ .nodeIndex = nodeIt->second };
				for (uint32_t i = 0; i < sourceTrack->mNumScalingKeys; ++i) {
					const auto& key = sourceTrack->mScalingKeys[i];
					track.scales.push_back({ static_cast<float>(key.mTime / ticksPerSecond), { key.mValue.x, key.mValue.y, key.mValue.z } });
				}
				for (uint32_t i = 0; i < sourceTrack->mNumRotationKeys; ++i) {
					const auto& key = sourceTrack->mRotationKeys[i];
					track.rotations.push_back({ static_cast<float>(key.mTime / ticksPerSecond), { key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w } });
				}
				for (uint32_t i = 0; i < sourceTrack->mNumPositionKeys; ++i) {
					const auto& key = sourceTrack->mPositionKeys[i];
					track.translations.push_back({
						static_cast<float>(key.mTime / ticksPerSecond),
						{ key.mValue.x, key.mValue.y, key.mValue.z },
					});
				}
				clip.tracks.push_back(std::move(track));
			}
			const std::string baseName = clip.name;
			uint32_t duplicateIndex = 1;
			while (model->rig->animationIndices.contains(clip.name)) {
				clip.name = std::format("{}#{}", baseName, duplicateIndex++);
			}
			model->rig->animationIndices.emplace(clip.name, model->rig->animations.size());
			model->rig->animations.push_back(std::move(clip));
		}
	}

	if (model->meshes.empty()) {
		throw std::runtime_error("Model '" + debugName + "': no drawable triangle meshes were found.");
	}

	return models_.emplace(path.native(), std::move(model)).first->second;
}
