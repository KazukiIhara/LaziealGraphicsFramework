#include "ModelManager.h"

#include <cstring>
#include <filesystem>
#include <format>
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
		return textureManager.Load(ToUtf8String(texturePath), loadDesc).srvIndex;
	}

	constexpr size_t kMaxMeshletVertices = 128;
	constexpr size_t kMaxMeshletPrimitives = 128;
	static_assert(sizeof(DirectX::Meshlet) == 16);
	static_assert(sizeof(DirectX::MeshletTriangle) == sizeof(uint32_t));
	static_assert(sizeof(DirectX::CullData) == 24);

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

std::shared_ptr<ModelResource> ModelManager::Load(const std::string& filePath) {
	if (filePath.empty()) {
		throw std::invalid_argument("Model path must not be empty.");
	}

	const auto path = std::filesystem::absolute(ToPath(filePath)).lexically_normal();
	if (const auto it = models_.find(path.native()); it != models_.end()) {
		return it->second;
	}

	Assimp::Importer importer;
	constexpr unsigned int kImportFlags =
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_PreTransformVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_ConvertToLeftHanded |
		aiProcess_ValidateDataStructure;
	const aiScene* scene = importer.ReadFile(ToUtf8String(path), kImportFlags);
	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0 || !scene->mRootNode) {
		throw std::runtime_error(std::format(
			"Model '{}': Assimp import failed: {}",
			filePath,
			importer.GetErrorString()));
	}

	auto model = std::make_shared<ModelResource>();
	model->filePath = filePath;
	model->meshes.reserve(scene->mNumMeshes);
	uint32_t meshletIndexOffset = 0;

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		const aiMesh* sourceMesh = scene->mMeshes[meshIndex];
		if (!sourceMesh || sourceMesh->mNumVertices == 0 || sourceMesh->mNumFaces == 0) {
			continue;
		}
		if (!sourceMesh->HasNormals()) {
			throw std::runtime_error(std::format(
				"Model '{}': mesh {} has no normals.",
				filePath,
				meshIndex));
		}

		std::vector<ModelVertex> vertices(sourceMesh->mNumVertices);
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

		std::vector<uint32_t> indices;
		indices.reserve(static_cast<size_t>(sourceMesh->mNumFaces) * 3);
		for (uint32_t faceIndex = 0; faceIndex < sourceMesh->mNumFaces; ++faceIndex) {
			const aiFace& face = sourceMesh->mFaces[faceIndex];
			if (face.mNumIndices != 3) {
				throw std::runtime_error(std::format(
					"Model '{}': mesh {} contains a non-triangle face after triangulation.",
					filePath,
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
				filePath,
				meshIndex,
				static_cast<uint32_t>(meshletHr)));
		}
		if (meshlets.empty() || meshletVertexIndexBytes.empty() || meshletPrimitives.empty()) {
			throw std::runtime_error(std::format(
				"Model '{}': meshlet generation produced no drawable data for mesh {}.",
				filePath,
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
				filePath,
				meshIndex,
				static_cast<uint32_t>(cullDataHr)));
		}

		ModelMeshResource mesh{};
		mesh.vertexResource = CreateUploadBuffer(directX_, vertices);
		mesh.meshletResource = CreateUploadBuffer(directX_, meshlets);
		mesh.meshletVertexIndexResource = CreateUploadBuffer(directX_, meshletVertexIndices);
		mesh.meshletPrimitiveResource = CreateUploadBuffer(directX_, meshletPrimitives);
		mesh.meshletCullDataResource = CreateUploadBuffer(directX_, meshletCullData);
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
		}

		model->meshes.push_back(std::move(mesh));
	}

	if (model->meshes.empty()) {
		throw std::runtime_error("Model '" + filePath + "': no drawable triangle meshes were found.");
	}

	return models_.emplace(path.native(), std::move(model)).first->second;
}
