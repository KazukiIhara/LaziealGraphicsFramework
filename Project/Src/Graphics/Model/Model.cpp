#include "LGF/Graphics/Model.h"

#include <cassert>
#include <stdexcept>

#include "LGF/Graphics/Graphics3D.h"
#include "ModelManager.h"
#include "ModelSystemInternal.h"

namespace {
	LGF::ModelManager* g_modelManager = nullptr;
}

void LGF::ModelSystem::Internal::SetModelManager(ModelManager* modelManager) {
	assert(modelManager != nullptr);
	g_modelManager = modelManager;
}

void LGF::ModelSystem::Internal::ResetModelManager() {
	g_modelManager = nullptr;
}

LGF::Model::Model(const std::string& filePath) :
	filePath_(filePath) {

	if (!g_modelManager) {
		throw std::runtime_error("Model must be loaded after framework initialization.");
	}
	resource_ = g_modelManager->Load(filePath);
}

LGF::Model::~Model() = default;

void LGF::Model::Draw(
	const Matrix4x4& worldMatrix,
	const Vector4& color,
	const ModelDebugDraw& debugDraw) const {

	if (!resource_) {
		throw std::runtime_error("Cannot draw an empty Model.");
	}
	Graphics3D::Draw(*this, worldMatrix, color, debugDraw);
}

const std::string& LGF::Model::GetFilePath() const {
	return filePath_;
}

bool LGF::Model::IsValid() const {
	return resource_ != nullptr;
}
