#pragma once

#include <cstdint>

#include <d3d12.h>

#include "ComPtr/ComPtr.h"
#include "LGF/Graphics/BlendMode.h"

namespace LGF {

	/// <summary>
	/// 3D描画クラスのインターフェース
	/// </summary>
	class IDrawer3D {
	public:
		virtual ~IDrawer3D() = default;

		/// <summary>
		/// リソースの作成など初期化処理
		/// </summary>
		virtual void Initialize() = 0;

		/// <summary>
		/// カウント変数のリセット
		/// </summary>
		virtual void BeginFrame() = 0;

		/// <summary>
		/// 積んだコマンドの実行
		/// </summary>
		virtual void EndFrame() = 0;

	private:
		// インスタンシング描画用のリソース
		ComPtr<ID3D12Resource> instanceResource_[Graphics::kBlendModeNum];

		// マテリアルのリソース
		ComPtr<ID3D12Resource> materialReource_[Graphics::kBlendModeNum];

		// インスタンスリソースのsrvインデックス
		uint32_t instanceSrvIndex_[Graphics::kBlendModeNum];

		// マテリアルリソースのsrvインデックス
		uint32_t materialSrvIndex_[Graphics::kBlendModeNum];

		// インスタンスのカウント
		uint32_t instanceCount_[Graphics::kBlendModeNum];

		// インスタンス追加用インデックス
		uint32_t currentIndex_[Graphics::kBlendModeNum];

	};

}