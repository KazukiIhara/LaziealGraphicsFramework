#include "LGF/Math/Matrix3x3.h"

#include <cstdint>

namespace LGF {

	Matrix3x3& Matrix3x3::operator+=(const Matrix3x3& mat) {
		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				m[y][x] += mat.m[y][x];
			}
		}

		return *this;
	}

	Matrix3x3& Matrix3x3::operator-=(const Matrix3x3& mat) {
		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				m[y][x] -= mat.m[y][x];
			}
		}

		return *this;
	}

	Matrix3x3& Matrix3x3::operator*=(float s) {
		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				m[y][x] *= s;
			}
		}

		return *this;
	}

	Matrix3x3& Matrix3x3::operator/=(float s) {
		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				m[y][x] /= s;
			}
		}

		return *this;
	}

	Matrix3x3 operator-(const Matrix3x3& mat) {
		Matrix3x3 result{};

		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				result.m[y][x] = -mat.m[y][x];
			}
		}

		return result;
	}

	Matrix3x3 operator+(const Matrix3x3& mat) {
		return mat;
	}

	Matrix3x3 operator+(const Matrix3x3& lhs, const Matrix3x3& rhs) {
		Matrix3x3 result{};

		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				result.m[y][x] = lhs.m[y][x] + rhs.m[y][x];
			}
		}

		return result;
	}

	Matrix3x3 operator-(const Matrix3x3& lhs, const Matrix3x3& rhs) {
		Matrix3x3 result{};

		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				result.m[y][x] = lhs.m[y][x] - rhs.m[y][x];
			}
		}

		return result;
	}

	Matrix3x3 operator*(const Matrix3x3& m1, const Matrix3x3& m2) {
		Matrix3x3 result{};

		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				result.m[y][x] =
					m1.m[y][0] * m2.m[0][x] +
					m1.m[y][1] * m2.m[1][x] +
					m1.m[y][2] * m2.m[2][x];
			}
		}

		return result;
	}

	Matrix3x3 operator*(const Matrix3x3& mat, float s) {
		Matrix3x3 result{};

		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				result.m[y][x] = mat.m[y][x] * s;
			}
		}

		return result;
	}

	Matrix3x3 operator*(float s, const Matrix3x3& mat) {
		return mat * s;
	}

	Matrix3x3 operator/(const Matrix3x3& mat, float s) {
		Matrix3x3 result{};

		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				result.m[y][x] = mat.m[y][x] / s;
			}
		}

		return result;
	}

	bool operator==(const Matrix3x3& lhs, const Matrix3x3& rhs) {
		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				if (lhs.m[y][x] != rhs.m[y][x]) {
					return false;
				}
			}
		}

		return true;
	}

	bool operator!=(const Matrix3x3& lhs, const Matrix3x3& rhs) {
		return !(lhs == rhs);
	}
}