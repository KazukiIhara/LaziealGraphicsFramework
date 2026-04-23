#pragma once

#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "BaseViewManager/BaseViewManager.h"

namespace LGF {

	class Device;
	class DescriptorHeapFactory;

	/// <summary>
	/// RTV管理クラス
	/// </summary>
	class RTVManager : public BaseViewManager {
	public:
		// コンストラクタ
		RTVManager(Device& device, DescriptorHeapFactory& descriptorHeapManager);
		// デストラクタ
		~RTVManager() override = default;

		// 初期化
		void Initialize(uint32_t maxViewCount = 64u);
		// Texture2D用のRTVを作成
		void CreateRTVTexture2d(uint32_t rtvIndex, ID3D12Resource* resource, DXGI_FORMAT format);

	private:
		// Device
		Device& device_;
	};
}
