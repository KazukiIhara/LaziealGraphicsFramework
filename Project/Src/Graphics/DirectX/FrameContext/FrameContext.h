#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"

namespace LGF {

	inline constexpr uint32_t kFrameContextCount = 2u;

	struct FrameContext {
		ComPtr<ID3D12CommandAllocator> commandAllocator;
		uint64_t fenceValue = 0u;
	};

	class FrameBufferedUploadBuffer {
	public:
		FrameBufferedUploadBuffer() = default;
		~FrameBufferedUploadBuffer() { Finalize(); }

		FrameBufferedUploadBuffer(const FrameBufferedUploadBuffer&) = delete;
		FrameBufferedUploadBuffer& operator=(const FrameBufferedUploadBuffer&) = delete;
		FrameBufferedUploadBuffer(FrameBufferedUploadBuffer&&) = delete;
		FrameBufferedUploadBuffer& operator=(FrameBufferedUploadBuffer&&) = delete;

		bool Initialize(ResourceFactory& factory, size_t sizeInBytes) {
			Finalize();
			for (uint32_t frameIndex = 0u; frameIndex < kFrameContextCount; ++frameIndex) {
				resources_[frameIndex] = factory.CreateUploadBufferResource(sizeInBytes);
				if (!resources_[frameIndex]) {
					Finalize();
					return false;
				}
				void* mappedData = nullptr;
				const HRESULT result = resources_[frameIndex]->Map(0, nullptr, &mappedData);
				if (FAILED(result) || !mappedData) {
					Finalize();
					return false;
				}
				mappedData_[frameIndex] = static_cast<std::byte*>(mappedData);
			}
			return true;
		}

		void Finalize() {
			for (uint32_t frameIndex = 0u; frameIndex < kFrameContextCount; ++frameIndex) {
				if (resources_[frameIndex] && mappedData_[frameIndex]) {
					resources_[frameIndex]->Unmap(0, nullptr);
				}
				mappedData_[frameIndex] = nullptr;
				resources_[frameIndex].Reset();
			}
		}

		ID3D12Resource* GetResource(uint32_t frameIndex) const {
			return resources_.at(frameIndex).Get();
		}

		std::byte* GetMappedData(uint32_t frameIndex) const {
			return mappedData_.at(frameIndex);
		}

		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress(uint32_t frameIndex) const {
			ID3D12Resource* resource = GetResource(frameIndex);
			return resource ? resource->GetGPUVirtualAddress() : 0u;
		}

	private:
		std::array<ComPtr<ID3D12Resource>, kFrameContextCount> resources_{};
		std::array<std::byte*, kFrameContextCount> mappedData_{};
	};

}
