#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <memory>

#include <d3d12.h>

namespace LGF {

	class DirectXCore;
	class Camera3D;
	class Camera3DResource;

	/// <summary>
	/// カメラ3Dシステム
	/// </summary>
	class Camera3DSystem {
	public:
		Camera3DSystem(DirectXCore& directX);
		~Camera3DSystem();

		void Initialize();
		void Finalize();

		void BeginFrame();
		void EndFrame();

		void Add(const std::string& name, std::unique_ptr<Camera3D> camera);
		void Remove(const std::string& name);

		void Set(const std::string& name);
		Camera3D* Get(const std::string& name);

		void Bind(ID3D12GraphicsCommandList* commandList,uint32_t rootParamIdx);

	private:
		// カメラマップ
		std::map<std::string, std::unique_ptr<Camera3D>> cameraMap_;
		// GPUに送るカメラリソース
		std::unique_ptr<Camera3DResource> resource_;
		// 使用するカメラの名前
		std::string currentCameraName_{};
	};
}