#pragma once

#include "LGF/Math/MathConstants.h"
#include "LGF/Math/MathTypes.h"

namespace LGF {

	/// <summary>
	/// カメラパラメータ
	/// </summary>
	struct CameraParam {
		float fovY = 45.0f * Math::DEG_TO_RAD;
		float aspectRatio = 16.0f / 9.0f;
		float nearClip = 0.1f;
		float farClip = 1000.0f;
	};

	/// <summary>
	/// 3Dカメラ
	/// </summary>
	class Camera3D {
	public:
		Camera3D();

		Camera3D(
			Vector3 translation,
			float yaw = {},
			float pitch = {},
			const CameraParam& param = {});

		~Camera3D();

		void Set()const;

	public:
		Vector3 translation{};
		float yaw{};
		float pitch{};
		CameraParam param;
	};

}