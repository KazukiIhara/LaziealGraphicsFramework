#pragma once

namespace LGF {

	class ModelManager;

	namespace ModelSystem::Internal {
		void SetModelManager(ModelManager* modelManager);
		void ResetModelManager();
	}

}
