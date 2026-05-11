#pragma once


namespace LGF {

	class FrameworkContext;

	/// <summary>
	/// フレームワークシステムのインターフェース
	/// </summary>
	class IFrameworkSystem {
	public:
		virtual ~IFrameworkSystem() = default;

		virtual bool Initialize(FrameworkContext& context) = 0;
		virtual void Finalize(FrameworkContext& context) = 0;

		virtual void BeginFrame(FrameworkContext& context) {};
		virtual void Update(FrameworkContext& context) {};
		virtual void EndFrame(FrameworkContext& context) {};
	};
}