#include "Renderer3D.h"

#include <algorithm>

#include "Graphics/Drawer/IDrawer.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"

#include "Graphics/Drawer/3D/Primitive/Plane/PlaneDrawer3D.h"
#include "Graphics/Drawer/3D/Primitive/Triangle/TriangleDrawer3D.h"
#include "LGF/Graphics/Plane3D.h"
#include "LGF/Graphics/Triangle3D.h"

#include "LGF/Math/MathUtils.h"

using namespace LGF;

Renderer3D::Renderer3D(Graphics3DContext context) :
	context_(context) {
}

Renderer3D::~Renderer3D() = default;

bool Renderer3D::Initialize() {
	// レンダラーを作成
	planeDrawer_ = std::make_unique<PlaneDrawer3D>(context_);
	planeDrawer_->Initialize();
	triangleDrawer_ = std::make_unique<TriangleDrawer3D>(context_);
	triangleDrawer_->Initialize();
	// 更新用リストに追加
	drawerList_.push_back(planeDrawer_.get());
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
	planeDrawer_.reset();
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
