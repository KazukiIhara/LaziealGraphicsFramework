#pragma once

#include "LGF/Math/MathConstants.h"
#include "LGF/Math/MathTypes.h"

namespace LGF {

	/// <summary>
	/// 3Dカメラ
	/// </summary>
	class Camera3D {
	public:
		/// <summary>
		/// カメラパラメータ
		/// </summary>
		struct Param {
			float fovY = 45.0f * Math::DEG_TO_RAD;
			bool useWindowAspectRatio = true;
			float aspectRatio = 16.0f / 9.0f;
			float nearClip = 0.1f;
			float farClip = 1000.0f;
		};
		/// <summary>
		/// デフォルトの姿勢のカメラを作成
		/// </summary>
		Camera3D();
		/// <summary>
		/// 姿勢とパラメータを指定してカメラを作成
		/// </summary>
		Camera3D(
			Vector3 translation,
			float yaw = {},
			float pitch = {},
			const Param& param = {});

		virtual ~Camera3D();

		void SetScene()const;

	public:
		Vector3 translation{};
		float yaw{};
		float pitch{};
		Param param{};
	};

}
