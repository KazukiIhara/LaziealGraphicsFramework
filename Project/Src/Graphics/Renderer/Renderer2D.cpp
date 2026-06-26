#include "Renderer2D.h"

#include "Graphics/Drawer/IDrawer.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"

#include "Graphics/Drawer/2D/Primitive/Triangle/TriangleDrawer2D.h"
#include "LGF/Graphics/Triangle2D.h"

#include "LGF/Math/MathUtils.h"

using namespace LGF;

Renderer2D::Renderer2D(Graphics2DContext context) :
	context_(context) {
}

Renderer2D::~Renderer2D() = default;

bool Renderer2D::Initialize() {
	return false;
}

void Renderer2D::BeginFrame() {
}

void Renderer2D::EndFrame() {
	for (const auto& drawer : drawerList_) {
		drawer->EndFrame();
	}
}

void Renderer2D::Finalize() {
}

