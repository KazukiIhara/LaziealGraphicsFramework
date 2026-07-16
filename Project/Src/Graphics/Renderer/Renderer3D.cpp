#include "Renderer3D.h"

#include <algorithm>

#include "Graphics/Drawer/IDrawer.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"

#include "Graphics/Drawer/3D/Primitive/Box/BoxDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Ground/GroundDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Line/LineDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Plane/PlaneDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Sphere/SphereDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Triangle/TriangleDrawer3D.h"
#include "LGF/Graphics/Box3D.h"
#include "LGF/Graphics/Ground3D.h"
#include "LGF/Graphics/Line3D.h"
#include "LGF/Graphics/Plane3D.h"
#include "LGF/Graphics/Sphere3D.h"
#include "LGF/Graphics/Triangle3D.h"

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
	groundDrawer_ = std::make_unique<GroundDrawer3D>(context_);
	groundDrawer_->Initialize();
	lineDrawer_ = std::make_unique<LineDrawer3D>(context_);
	lineDrawer_->Initialize();
	planeDrawer_ = std::make_unique<PlaneDrawer3D>(context_);
	planeDrawer_->Initialize();
	sphereDrawer_ = std::make_unique<SphereDrawer3D>(context_);
	sphereDrawer_->Initialize();
	triangleDrawer_ = std::make_unique<TriangleDrawer3D>(context_);
	triangleDrawer_->Initialize();
	// 更新用リストに追加
	drawerList_.push_back(boxDrawer_.get());
	drawerList_.push_back(groundDrawer_.get());
	drawerList_.push_back(lineDrawer_.get());
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
	groundDrawer_.reset();
	lineDrawer_.reset();
	planeDrawer_.reset();
	sphereDrawer_.reset();
	triangleDrawer_.reset();
}

void Renderer3D::AddTriangle(const Triangle3D& triangle, const Matrix4x4& worldMatrix, const Vector4& color) {
	Triangle3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	data.instance.baseColor = color;

	std::ranges::copy(
		triangle.vertices,
		data.vertexOffsets
	);

	triangleDrawer_->AddTriangle(data);
}

void Renderer3D::AddBox(const Box3D& box, const Matrix4x4& worldMatrix, const Vector4& color) {
	Box3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	data.instance.baseColor = color;

	data.halfSize = {
		box.width * 0.5f,
		box.height * 0.5f,
		box.depth * 0.5f
	};

	boxDrawer_->AddBox(data);
}

void Renderer3D::AddLine(const Line3D& line, const Matrix4x4& worldMatrix, const Vector4& color) {
	Line3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	data.instance.baseColor = color;

	data.start = line.start;
	data.end = line.end;

	lineDrawer_->AddLine(data);
}

void Renderer3D::AddPlane(const Plane3D& plane, const Matrix4x4& worldMatrix, const Vector4& color) {
	Plane3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	data.instance.baseColor = color;

	const float halfWidth = plane.width * 0.5f;
	const float halfHeight = plane.height * 0.5f;

	data.vertexOffsets[0] = { -halfWidth, -halfHeight, 0.0f };
	data.vertexOffsets[1] = {  halfWidth, -halfHeight, 0.0f };
	data.vertexOffsets[2] = { -halfWidth,  halfHeight, 0.0f };
	data.vertexOffsets[3] = {  halfWidth,  halfHeight, 0.0f };

	planeDrawer_->AddPlane(data);
}

void Renderer3D::AddGround(const Ground3D& ground, const Matrix4x4& worldMatrix, const Vector4& color) {
	Ground3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	data.instance.baseColor = color;

	const float halfWidth = ground.width * 0.5f;
	const float halfDepth = ground.depth * 0.5f;

	data.vertexOffsets[0] = { -halfWidth, 0.0f, -halfDepth };
	data.vertexOffsets[1] = { -halfWidth, 0.0f,  halfDepth };
	data.vertexOffsets[2] = {  halfWidth, 0.0f, -halfDepth };
	data.vertexOffsets[3] = {  halfWidth, 0.0f,  halfDepth };

	groundDrawer_->AddGround(data);
}

void Renderer3D::AddSphere(const Sphere3D& sphere, const Matrix4x4& worldMatrix, const Vector4& color) {
	Sphere3DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.worldInverseTranspose =
		Math::Transpose(Math::Inverse(data.instance.worldMatrix));

	data.instance.baseColor = color;
	data.radius = sphere.radius;

	sphereDrawer_->AddSphere(data);
}
