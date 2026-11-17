#pragma once

#include <cstdint>
#include <filesystem>
#include <limits>

#include "LGF/Math/Matrix4x4.h"

namespace LGF {

	struct EffectWorldHandle3D {
		static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

		uint32_t index = InvalidIndex;
		uint32_t generation{};

		bool IsValid() const;
		bool operator==(const EffectWorldHandle3D&) const = default;
	};

	struct EffectResourceHandle3D {
		static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

		uint32_t index = InvalidIndex;
		uint32_t generation{};
		EffectWorldHandle3D world{};

		bool IsValid() const;
		bool operator==(const EffectResourceHandle3D&) const = default;
	};

	struct EffectInstanceHandle3D {
		static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

		uint32_t index = InvalidIndex;
		uint32_t generation{};
		EffectWorldHandle3D world{};

		bool IsValid() const;
		bool operator==(const EffectInstanceHandle3D&) const = default;
	};

	namespace Effect3D {
		EffectWorldHandle3D CreateWorld(uint32_t maximumInstanceCount);
		bool RemoveWorld(EffectWorldHandle3D handle);
		EffectResourceHandle3D Load(
			EffectWorldHandle3D world,
			const std::filesystem::path& filePath);
		bool IsValid(EffectResourceHandle3D handle);
		bool Remove(EffectResourceHandle3D handle);
		EffectInstanceHandle3D Play(
			EffectResourceHandle3D effect,
			const Matrix4x4& transform);
		bool Stop(EffectInstanceHandle3D handle);
		bool IsPlaying(EffectInstanceHandle3D handle);
		bool SetTransform(
			EffectInstanceHandle3D handle,
			const Matrix4x4& transform);
		bool Update(EffectWorldHandle3D handle, float deltaTime);
		bool Clear(EffectWorldHandle3D handle);
	}

}
