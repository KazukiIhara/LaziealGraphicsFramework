#include "LGF/Graphics/DebugCamera3D.h"

#include <algorithm>
#include <cmath>

#include "LGF/Input.h"
#include "LGF/Math/MathConstants.h"

using namespace LGF;

namespace {

	constexpr float kInitialPitch = 15.0f * Math::DEG_TO_RAD;
	constexpr float kPitchLimit = 89.0f * Math::DEG_TO_RAD;
	constexpr float kMinimumDistance = 0.1f;
	constexpr float kMaximumDeltaTime = 0.1f;

}

DebugCamera3D::DebugCamera3D(float distance, const Camera3D::Param& param) :
	Camera3D({}, 0.0f, kInitialPitch, param),
	distance(std::max(distance, kMinimumDistance)),
	previousUpdateTime_(std::chrono::steady_clock::now()) {
	UpdateTranslation();
}

DebugCamera3D::~DebugCamera3D() = default;

void DebugCamera3D::Update() {
	const auto currentTime = std::chrono::steady_clock::now();
	const float deltaTime = std::min(
		std::chrono::duration<float>(currentTime - previousUpdateTime_).count(),
		kMaximumDeltaTime);
	previousUpdateTime_ = currentTime;

	const float horizontalInput =
		static_cast<float>(Key::Left.Press()) - static_cast<float>(Key::Right.Press());
	const float verticalInput =
		static_cast<float>(Key::Up.Press()) - static_cast<float>(Key::Down.Press());

	yaw = std::remainder(yaw + horizontalInput * rotationSpeed * deltaTime, Math::TWO_PI);
	pitch = std::clamp(
		pitch + verticalInput * rotationSpeed * deltaTime,
		-kPitchLimit,
		kPitchLimit);
	distance = std::max(distance, kMinimumDistance);

	UpdateTranslation();
}

void DebugCamera3D::UpdateTranslation() {
	const float cosPitch = std::cos(pitch);
	const Vector3 forward{
		std::sin(yaw) * cosPitch,
		-std::sin(pitch),
		std::cos(yaw) * cosPitch,
	};

	// 注視点は原点なので、前方向ベクトルと逆向きに配置する。
	translation = -forward * distance;
}
