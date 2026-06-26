#include "BaseDrawer3D.h"

using namespace LGF;

BaseDrawer3D::BaseDrawer3D(Graphics3DContext context) :
	BaseDrawer(context.directX, context.pipelineMgr), context_(context) {
}

BaseDrawer3D::~BaseDrawer3D() = default;

Graphics3DContext BaseDrawer3D::GetContext() const {
	return context_;
}