#pragma once

#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "BaseViewManager/BaseViewManager.h"

namespace LGF {

	class Device;
	class DescriptorHeapFactory;

	/// <summary>
	/// DSV管理クラス
	/// </summary>
	class DSVManager : public BaseViewManager {
	public:
		// コンストラクタ
		DSVManager(Device& device, DescriptorHeapFactory& descriptorHeapManager);
		// デストラクタ
		~DSVManager() override = default;

		// 初期化
		void Initialize(uint32_t maxViewCount = 8u);
		// Texture2D用のDSVを作成
		void CreateDSVTexture2d(uint32_t dsvIndex, ID3D12Resource* resource, DXGI_FORMAT format);

	private:
		// Device
		Device& device_;
	};
}
