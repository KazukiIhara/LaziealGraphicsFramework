#pragma once

#include "LGF/Math/MathTypes.h"

namespace LGF {

	/// <summary>
	/// 3Dカメラ
	/// </summary>
	class Camera3D {
	public:
		Camera3D();
		~Camera3D();

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
	};

}