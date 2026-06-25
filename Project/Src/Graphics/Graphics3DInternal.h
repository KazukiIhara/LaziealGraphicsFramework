#pragma once

namespace LGF {

	class Renderer3D;

	namespace Graphics3D::Internal {

		void SetRenderer(Renderer3D* renderer);
		void ResetRenderer();

	}

}