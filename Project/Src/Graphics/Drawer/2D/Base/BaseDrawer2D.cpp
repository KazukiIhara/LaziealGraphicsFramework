#include "BaseDrawer2D.h"

using namespace LGF;

BaseDrawer2D::BaseDrawer2D(Graphics2DContext context) :
	BaseDrawer(context.directX, context.pipelineMgr),
	context_(context) {
}

BaseDrawer2D::~BaseDrawer2D() = default;

Graphics2DContext BaseDrawer2D::GetContext() const {
	return context_;
}
