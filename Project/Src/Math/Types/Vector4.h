#pragma once


namespace LGF {
	/// <summary>
	/// 四次元ベクトル
	/// </summary>
	struct Vector4 final {
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float w = 0.0f;

		Vector4& operator+=(const Vector4& v) {
			x += v.x; y += v.y; z += v.z; w += v.w;
			return *this;
		}
		Vector4& operator-=(const Vector4& v) {
			x -= v.x; y -= v.y; z -= v.z; w -= v.w;
			return *this;
		}
		Vector4& operator*=(const Vector4& v) {
			x *= v.x; y *= v.y; z *= v.z; w *= v.w;
			return *this;
		}
		Vector4& operator/=(const Vector4& v) {
			x /= v.x; y /= v.y; z /= v.z; w /= v.w;
			return *this;
		}

		Vector4& operator*=(float s) {
			x *= s; y *= s; z *= s; w *= s;
			return *this;
		}
		Vector4& operator/=(float s) {
			x /= s; y /= s; z /= s; w /= s;
			return *this;
		}
	};
}