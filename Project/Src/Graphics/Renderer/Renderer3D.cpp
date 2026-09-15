#include "Renderer3D.h"

#include <algorithm>
#include <stdexcept>

#include "Graphics/Drawer/IDrawer.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"

#include "Graphics/Drawer/3D/Primitive/Box/BoxDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Capsule/CapsuleDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Cone/ConeDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Cylinder/CylinderDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Ground/GroundDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Line/LineDrawer3D.h"
#include "Graphics/Drawer/3D/Debug/MeshletBounds/MeshletBoundsDrawer3D.h"
#include "Graphics/Drawer/3D/Model/ModelDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Plane/PlaneDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Sphere/SphereDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Triangle/TriangleDrawer3D.h"
#include "LGF/Graphics/Box3D.h"
#include "LGF/Graphics/Capsule3D.h"
#include "LGF/Graphics/Cone3D.h"
#include "LGF/Graphics/Cylinder3D.h"
#include "LGF/Graphics/Ground3D.h"
#include "LGF/Graphics/Line3D.h"
#include "LGF/Graphics/Model.h"
#include "LGF/Graphics/Plane3D.h"
#include "LGF/Graphics/Sphere3D.h"
#include "LGF/Graphics/Triangle3D.h"

#include "Graphics/Model/ModelResource.h"

#include "LGF/Math/MathUtils.h"

using namespace LGF;

Renderer3D::Renderer3D(Graphics3DContext context) :
	context_(context) {
}

Renderer3D::~Renderer3D() = default;

bool Renderer3D::Initialize() {
	// レンダラーを作成
	boxDrawer_ = std::make_unique<BoxDrawer3D>(context_);
	boxDrawer_->Initialize();
	capsuleDrawer_ = std::make_unique<CapsuleDrawer3D>(context_);
	capsuleDrawer_->Initialize();
	coneDrawer_ = std::make_unique<ConeDrawer3D>(context_);
	coneDrawer_->Initialize();
	cylinderDrawer_ = std::make_unique<CylinderDrawer3D>(context_);
	cylinderDrawer_->Initialize();
	groundDrawer_ = std::make_unique<GroundDrawer3D>(context_);
	groundDrawer_->Initialize();
	lineDrawer_ = std::make_unique<LineDrawer3D>(context_);
	lineDrawer_->Initialize();
	meshletBoundsDrawer_ = std::make_unique<MeshletBoundsDrawer3D>(context_);
	meshletBoundsDrawer_->Initialize();
	modelDrawer_ = std::make_unique<ModelDrawer3D>(context_);
	modelDrawer_->Initialize();
	planeDrawer_ = std::make_unique<PlaneDrawer3D>(context_);
	planeDrawer_->Initialize();
	sphereDrawer_ = std::make_unique<SphereDrawer3D>(context_);
	sphereDrawer_->Initialize();
	triangleDrawer_ = std::make_unique<TriangleDrawer3D>(context_);
	triangleDrawer_->Initialize();
	// 更新用リストに追加
	drawerList_.push_back(boxDrawer_.get());
	drawerList_.push_back(capsuleDrawer_.get());
	drawerList_.push_back(coneDrawer_.get());
	drawerList_.push_back(cylinderDrawer_.get());
	drawerList_.push_back(groundDrawer_.get());
	drawerList_.push_back(lineDrawer_.get());
	drawerList_.push_back(modelDrawer_.get());
	drawerList_.push_back(meshletBoundsDrawer_.get());
	drawerList_.push_back(planeDrawer_.get());
	drawerList_.push_back(sphereDrawer_.get());
	drawerList_.push_back(triangleDrawer_.get());

	return true;
}

void Renderer3D::BeginFrame() {
}

void Renderer3D::EndFrame() {
	for (const auto& drawer : drawerList_) {
		drawer->EndFrame();
	}
}

void Renderer3D::Finalize() {
	boxDrawer_.reset();
	capsuleDrawer_.reset();
	coneDrawer_.reset();
	cylinderDrawer_.reset();
	groundDrawer_.reset();
	lineDrawer_.reset();
	meshletBoundsDrawer_.reset();
	modelDrawer_.reset();
	planeDrawer_.reset();
	sphereDrawer_.reset();
	triangleDrawer_.reset();
}

void Renderer3D::AddTriangle(const Triangle3D& triangle, const Matrix4x4& worldMatrix, const Material3D& material) {
	Triangle3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	ApplyMaterial(data.instance, material);

	std::ranges::copy(
		triangle.vertices,
		data.vertexOffsets
	);

	triangleDrawer_->AddTriangle(data);
}

void Renderer3D::AddBox(const Box3D& box, const Matrix4x4& worldMatrix, const Material3D& material) {
	Box3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	ApplyMaterial(data.instance, material);

	data.halfSize = {
		box.width * 0.5f,
		box.height * 0.5f,
		box.depth * 0.5f
	};

	boxDrawer_->AddBox(data);
}

void Renderer3D::AddCapsule(const Capsule3D& capsule, const Matrix4x4& worldMatrix, const Material3D& material) {
	Capsule3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	ApplyMaterial(data.instance, material);
	data.radius = capsule.radius;
	data.height = capsule.height;

	capsuleDrawer_->AddCapsule(data);
}

void Renderer3D::AddCone(const Cone3D& cone, const Matrix4x4& worldMatrix, const Material3D& material) {
	Cone3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	ApplyMaterial(data.instance, material);
	data.radius = cone.radius;
	data.height = cone.height;

	coneDrawer_->AddCone(data);
}

void Renderer3D::AddCylinder(const Cylinder3D& cylinder, const Matrix4x4& worldMatrix, const Material3D& material) {
	Cylinder3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	ApplyMaterial(data.instance, material);
	data.radius = cylinder.radius;
	data.height = cylinder.height;

	cylinderDrawer_->AddCylinder(data);
}

void Renderer3D::AddLine(const Line3D& line, const Matrix4x4& worldMatrix, const Material3D& material) {
	Line3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	ApplyMaterial(data.instance, material);

	data.start = line.start;
	data.end = line.end;

	lineDrawer_->AddLine(data);
}

void Renderer3D::AddPlane(const Plane3D& plane, const Matrix4x4& worldMatrix, const Material3D& material) {
	Plane3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	ApplyMaterial(data.instance, material);

	const float halfWidth = plane.width * 0.5f;
	const float halfHeight = plane.height * 0.5f;

	data.vertexOffsets[0] = { -halfWidth, -halfHeight, 0.0f };
	data.vertexOffsets[1] = {  halfWidth, -halfHeight, 0.0f };
	data.vertexOffsets[2] = { -halfWidth,  halfHeight, 0.0f };
	data.vertexOffsets[3] = {  halfWidth,  halfHeight, 0.0f };

	planeDrawer_->AddPlane(data);
}

void Renderer3D::AddGround(const Ground3D& ground, const Matrix4x4& worldMatrix, const Material3D& material) {
	Ground3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	ApplyMaterial(data.instance, material);

	const float halfWidth = ground.width * 0.5f;
	const float halfDepth = ground.depth * 0.5f;

	data.vertexOffsets[0] = { -halfWidth, 0.0f, -halfDepth };
	data.vertexOffsets[1] = { -halfWidth, 0.0f,  halfDepth };
	data.vertexOffsets[2] = {  halfWidth, 0.0f, -halfDepth };
	data.vertexOffsets[3] = {  halfWidth, 0.0f,  halfDepth };

	groundDrawer_->AddGround(data);
}

void Renderer3D::AddSphere(const Sphere3D& sphere, const Matrix4x4& worldMatrix, const Material3D& material) {
	Sphere3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	ApplyMaterial(data.instance, material);
	data.radius = sphere.radius;

	sphereDrawer_->AddSphere(data);
}

void Renderer3D::AddModel(
	const Model& model,
	const Matrix4x4& worldMatrix,
	const Vector4& color,
	const ModelDrawOptions& options) {

	const float worldMaximumScale = std::max({
		Math::Length(Vector3{ worldMatrix.m[0][0], worldMatrix.m[0][1], worldMatrix.m[0][2] }),
		Math::Length(Vector3{ worldMatrix.m[1][0], worldMatrix.m[1][1], worldMatrix.m[1][2] }),
		Math::Length(Vector3{ worldMatrix.m[2][0], worldMatrix.m[2][1], worldMatrix.m[2][2] }),
	});

	modelDrawer_->AddModel(
		model.resource_,
		worldMatrix,
		Math::Transpose(Math::Inverse(worldMatrix)),
		color,
		worldMaximumScale,
		Math::Determinant(worldMatrix) < 0.0f ? -1.0f : 1.0f,
		options.debugDraw.showMeshletColors,
		options.enableNormalMap,
		options.normalScale);

	if (!options.debugDraw.showMeshletBoundingSpheres) {
		return;
	}

	for (const ModelMeshResource& mesh : model.resource_->meshes) {
		for (const ModelMeshletBounds& bounds : mesh.meshletBounds) {
			MeshletBounds3DForGPU data{};
			const Vector3 worldCenter = Math::Transform(bounds.center, worldMatrix);
			data.instance.worldMatrix = Math::MakeTranslateMatrix(worldCenter);
			data.instance.worldInverseTranspose =
				Math::Transpose(Math::Inverse(data.instance.worldMatrix));
			data.instance.baseColor = options.debugDraw.meshletBoundingSphereColor;
			data.instance.textureIndex = UINT32_MAX;
			data.instance.normalTextureIndex = UINT32_MAX;
			data.radius = bounds.radius * worldMaximumScale;
			meshletBoundsDrawer_->AddBounds(data);
		}
	}
}

void Renderer3D::ApplyMaterial(InstanceData3D& instance, const Material3D& material) {
	if (material.baseColorTexture.isCubeMap_) {
		throw std::invalid_argument("Material3D baseColorTexture must be a Texture2D.");
	}
	if (material.normalTexture.isCubeMap_) {
		throw std::invalid_argument("Material3D normalTexture must be a Texture2D.");
	}
	instance.baseColor = material.color;
	instance.textureIndex = material.baseColorTexture.srvIndex_;
	instance.normalTextureIndex = material.normalTexture.srvIndex_;
	instance.normalScale = material.normalScale;
	instance.uvScale = material.uvScale;
	instance.uvOffset = material.uvOffset;
}
