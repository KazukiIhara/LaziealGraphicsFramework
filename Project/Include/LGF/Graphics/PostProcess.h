#pragma once

#include <cstdint>
#include <limits>

namespace LGF {

	/// <summary>
	/// 読み込んだPost Process Profileの実行時インスタンスを参照する世代付きハンドル
	/// </summary>
	struct PostProcessProfileHandle {
		static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

		uint32_t index = InvalidIndex;
		uint32_t generation{};

		bool IsValid() const {
			return index != InvalidIndex && generation != 0u;
		}
		bool operator==(const PostProcessProfileHandle&) const = default;
	};

}
