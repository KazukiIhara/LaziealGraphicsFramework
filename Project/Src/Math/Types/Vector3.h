#pragma once

namespace LGF {
	/// <summary>
	/// 三次元ベクトル
	/// </summary>
	struct Vector3 final {
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		Vector3& operator+=(const Vector3& v) {
			x += v.x; y += v.y; z += v.z;
			return *this;
		}
		Vector3& operator-=(const Vector3& v) {
			x -= v.x; y -= v.y; z -= v.z;
			return *this;
		}
		Vector3& operator*=(const Vector3& v) {
			x *= v.x; y *= v.y; z *= v.z;
			return *this;
		}
		Vector3& operator/=(const Vector3& v) {
			x -= v.x; y -= v.y; z -= v.z;
			return *this;
		}

		Vector3& operator*=(float s) {
			x *= s; y *= s; z *= s;
			return *this;
		}
		Vector3& operator/=(float s) {
			x /= s; y /= s; z /= s;
			return *this;
		}
	};
}