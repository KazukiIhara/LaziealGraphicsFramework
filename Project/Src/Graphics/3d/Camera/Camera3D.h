#pragma once

#include "LGF/Math/MathConstants.h"
#include "LGF/Math/MathTypes.h"

namespace LGF {

	/// <summary>
	/// 3Dカメラ
	/// </summary>
	class Camera3D {
	public:
		Camera3D();
		~Camera3D();

		void Initialize();

		void Update();

		void SetTranslation(const Vector3& translation);
		void SetYaw(float yaw);
		void SetPitch(float pitch);

		const Matrix4x4 GetViewMatrix()const;
		const Matrix4x4 GetProjectionMatrix()const;

	private:
		Vector3 translation_{};
		float yaw_{};
		float pitch_{};

		Quaternion rotation_{};

		Matrix4x4 viewMatrix_{};
		Matrix4x4 projectionMatrix_{};

		float fovY_ = 45.0f * Math::DEG_TO_RAD;
		float aspectRatio_ = 16.0f / 9.0f;
		float nearClip_ = 0.1f;
		float farClip_ = 1000.0f;
	};

}