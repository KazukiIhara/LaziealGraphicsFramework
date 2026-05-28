#pragma once

#include <memory>

#include "Framework/System/IFrameworkSystem.h"

#include "DirectX/Core/DirectXCore.h"
#include "DirectX/Renderer/TriangleRenderer.h"

namespace LGF {
	/// <summary>
	/// グラフィックシステムクラス
	/// </summary>
	class GraphicsSystem :public IFrameworkSystem {
	public:
		GraphicsSystem();
		~GraphicsSystem()override = default;

		bool Initialize(FrameworkContext& context)override;
		bool Update(FrameworkContext& context)override;
		bool Finalize(FrameworkContext& context)override;

		void BeginFrame(FrameworkContext& context)override;
		void EndFrame(FrameworkContext& context)override;

	private:
		std::unique_ptr<DirectXCore> directX_;
		std::unique_ptr<TriangleRenderer> triangleRenderer_;

	};
}
