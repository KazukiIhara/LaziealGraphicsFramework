#pragma once

#include "Core/Framework/FrameworkContext.h"

namespace LGF {

	/// <summary>
	/// フレームワークシステムのインターフェース
	/// </summary>
	class IFrameworkSystem {
	public:
		virtual ~IFrameworkSystem() = default;

		virtual bool Initialize(FrameworkContext& context) = 0;
		virtual bool Finalize(FrameworkContext& context) = 0;

		virtual void BeginFrame([[maybe_unused]] FrameworkContext& context) {};
		virtual void EndFrame([[maybe_unused]] FrameworkContext& context) {};
	};
}