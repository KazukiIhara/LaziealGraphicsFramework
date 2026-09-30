#pragma once

#include <cstdint>
#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"

#include "Graphics/Drawer/BaseDrawer.h"

#include "LGF/Math/Vector4.h"
#include "LGF/Math/Vector2.h"
#include "LGF/Math/Matrix4x4.h"

#include "Graphics/Graphics3DContext.h"

namespace LGF {
	class Pipeline;
	class ShadowMapSystem;

	/// <summary>
	/// 3Dインスタンシング描画用の構造体
	/// </summary>
	struct InstanceData3D {
		Matrix4x4 worldMatrix;
		Matrix4x4 worldInverseTranspose;
		Vector4 baseColor;
		Vector2 uvScale = { 1.0f, 1.0f };
		Vector2 uvOffset{};
		uint32_t textureIndex = UINT32_MAX;
		uint32_t normalTextureIndex = UINT32_MAX;
		float normalScale = 1.0f;
		uint32_t materialFlags = 0;
		uint32_t metallicRoughnessTextureIndex = UINT32_MAX;
		uint32_t occlusionTextureIndex = UINT32_MAX;
		float metallic = 0.0f;
		float roughness = 1.0f;
		float occlusionStrength = 1.0f;
		float padding[3]{};
	};
	static_assert(sizeof(InstanceData3D) == 208);

	/// <summary>
	/// 3D描画基底クラス
	/// </summary>
	class BaseDrawer3D :public BaseDrawer {
	public:
		BaseDrawer3D(
			Graphics3DContext context,
			uint32_t meshletCountPerInstance = 1u);
		virtual ~BaseDrawer3D()override;

		/// <summary>
		/// 描画クラス固有の初期化処理
		/// </summary>
		virtual void Initialize()override = 0;

		/// <summary>
		/// CPUで積んだコマンドをGPUに投げる
		/// </summary>
		virtual void EndFrame()override = 0;

		void DrawShadow(Pipeline* pipeline, ShadowMapSystem& shadowMap, const char* instanceBufferName);

	protected:
		/// <summary>
		/// 参照メンバ構造体を取得
		/// </summary>
		Graphics3DContext GetContext() const;

		uint32_t GetMeshletCountPerInstance() const;

	private:
		Graphics3DContext context_;
		uint32_t meshletCountPerInstance_ = 1u;
	};

}
