#pragma once


namespace LGF {

	/// <summary>
	/// グラフィックスシステムのサブシステム
	/// </summary>
	class IGraphicsSubsystem {
	public:
		virtual ~IGraphicsSubsystem() = default;

		virtual bool Initialize() = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
	};

}