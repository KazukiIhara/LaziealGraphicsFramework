#include "Matrix4x4.h"

#include <cstdint>

namespace LGF {

	Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& mat) {
		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				m[y][x] += mat.m[y][x];
			}
		}

		return *this;
	}

	Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& mat) {
		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				m[y][x] -= mat.m[y][x];
			}
		}

		return *this;
	}

	Matrix4x4& Matrix4x4::operator*=(float s) {
		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				m[y][x] *= s;
			}
		}

		return *this;
	}

	Matrix4x4& Matrix4x4::operator/=(float s) {
		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				m[y][x] /= s;
			}
		}

		return *this;
	}

	Matrix4x4 operator-(const Matrix4x4& mat) {
		Matrix4x4 result{};

		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				result.m[y][x] = -mat.m[y][x];
			}
		}

		return result;
	}

	Matrix4x4 operator+(const Matrix4x4& mat) {
		return mat;
	}

	Matrix4x4 operator+(const Matrix4x4& lhs, const Matrix4x4& rhs) {
		Matrix4x4 result{};

		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				result.m[y][x] = lhs.m[y][x] + rhs.m[y][x];
			}
		}

		return result;
	}

	Matrix4x4 operator-(const Matrix4x4& lhs, const Matrix4x4& rhs) {
		Matrix4x4 result{};

		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				result.m[y][x] = lhs.m[y][x] - rhs.m[y][x];
			}
		}

		return result;
	}

	Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) {
		Matrix4x4 result{};

		// 行列積
		for (uint32_t row = 0; row < 4; ++row) {
			for (uint32_t col = 0; col < 4; ++col) {
				for (uint32_t k = 0; k < 4; ++k) {
					result.m[row][col] += m1.m[row][k] * m2.m[k][col];
				}
			}
		}

		return result;
	}

	Matrix4x4 operator*(const Matrix4x4& mat, float s) {
		Matrix4x4 result{};

		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				result.m[y][x] = mat.m[y][x] * s;
			}
		}

		return result;
	}

	Matrix4x4 operator*(float s, const Matrix4x4& mat) {
		return mat * s;
	}

	Matrix4x4 operator/(const Matrix4x4& mat, float s) {
		Matrix4x4 result{};

		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				result.m[y][x] = mat.m[y][x] / s;
			}
		}

		return result;
	}

	bool operator==(const Matrix4x4& lhs, const Matrix4x4& rhs) {
		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				if (lhs.m[y][x] != rhs.m[y][x]) {
					return false;
				}
			}
		}

		return true;
	}

	bool operator!=(const Matrix4x4& lhs, const Matrix4x4& rhs) {
		return !(lhs == rhs);
	}
}