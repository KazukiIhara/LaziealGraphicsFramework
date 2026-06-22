#pragma once

#include <cstdint>
#include <string>
#include <d3d12.h>

#include "ComPtr/ComPtr.h"

#include "3d/Drawer/IDrawer3D.h"

#include "LGF/Math/Vector4.h"
#include "LGF/Math/Matrix4x4.h"

#include "3d/Graphics3DContext.h"

namespace LGF {

	class Pipeline;

	/// <summary>
	/// インスタンシング描画用の構造体
	/// </summary>
	struct InstanceData3D {
		Matrix4x4 worldMatrix;
		Matrix4x4 worldInverseTranspose;
		Vector4 baseColor;
	};

	/// <summary>
	/// 3D描画基底クラス
	/// </summary>
	class BaseDrawer3D :public IDrawer3D {
	protected:
		struct InitParam {
			uint32_t numMaxInstance;
			unsigned long long instResSize;
			void** instData;
		};
	public:
		BaseDrawer3D(Graphics3DContext context);
		virtual ~BaseDrawer3D();

		/// <summary>
		/// 描画クラス固有の初期化処理
		/// </summary>
		virtual void Initialize()override = 0;

		/// <summary>
		/// CPUで積んだコマンドをGPUに投げる
		/// </summary>
		virtual void EndFrame()override = 0;

	protected:
		/// <summary>
		/// 参照メンバ構造体を取得
		/// </summary>
		Graphics3DContext GetContext() const;

		/// <summary>
		/// パイプラインを取得
		/// </summary>
		Pipeline* GetPipeline();

		/// <summary>
		/// パイプラインをセット
		/// </summary>
		void SetPipeline(Pipeline* pipeline);

		/// <summary>
		/// リソースの作成など共通の初期化処理
		/// </summary>
		void Initialize(InitParam param);

		/// <summary>
		/// インスタンスリソースを作成
		/// </summary>
		void CreateInstanceResource(unsigned long long resSize, void** data,
			ComPtr<ID3D12Resource>& res, uint32_t& srvIdx);

	protected:
		// インスタンシング描画用のリソース
		ComPtr<ID3D12Resource> instanceResource_{};

		// インスタンスリソースのsrvインデックス
		uint32_t instanceSrvIndex_{};

		// インスタンスのカウント
		uint32_t instanceCount_{};
		// インスタンス追加用インデックス
		uint32_t currentIndex_{};

		// インスタンス最大数
		uint32_t numMaxInstance_{};

	private:
		Graphics3DContext context_;
		Pipeline* pipeline_ = nullptr;
	};

}