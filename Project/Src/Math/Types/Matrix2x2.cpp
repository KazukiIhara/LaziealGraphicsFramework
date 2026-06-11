#include "Matrix2x2.h"

#include <cstdint>

namespace LGF {

	Matrix2x2& Matrix2x2::operator+=(const Matrix2x2& mat) {
		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				m[y][x] += mat.m[y][x];
			}
		}

		return *this;
	}

	Matrix2x2& Matrix2x2::operator-=(const Matrix2x2& mat) {
		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				m[y][x] -= mat.m[y][x];
			}
		}

		return *this;
	}

	Matrix2x2& Matrix2x2::operator*=(float s) {
		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				m[y][x] *= s;
			}
		}

		return *this;
	}

	Matrix2x2& Matrix2x2::operator/=(float s) {
		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				m[y][x] /= s;
			}
		}

		return *this;
	}

	Matrix2x2 operator-(const Matrix2x2& mat) {
		Matrix2x2 result{};

		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				result.m[y][x] = -mat.m[y][x];
			}
		}

		return result;
	}

	Matrix2x2 operator+(const Matrix2x2& mat) {
		return mat;
	}

	Matrix2x2 operator+(const Matrix2x2& lhs, const Matrix2x2& rhs) {
		Matrix2x2 result{};

		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				result.m[y][x] = lhs.m[y][x] + rhs.m[y][x];
			}
		}

		return result;
	}

	Matrix2x2 operator-(const Matrix2x2& lhs, const Matrix2x2& rhs) {
		Matrix2x2 result{};

		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				result.m[y][x] = lhs.m[y][x] - rhs.m[y][x];
			}
		}

		return result;
	}

	Matrix2x2 operator*(const Matrix2x2& mat, float s) {
		Matrix2x2 result{};

		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				result.m[y][x] = mat.m[y][x] * s;
			}
		}

		return result;
	}

	Matrix2x2 operator*(float s, const Matrix2x2& mat) {
		return mat * s;
	}

	Matrix2x2 operator/(const Matrix2x2& mat, float s) {
		Matrix2x2 result{};

		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				result.m[y][x] = mat.m[y][x] / s;
			}
		}

		return result;
	}

	bool operator==(const Matrix2x2& lhs, const Matrix2x2& rhs) {
		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				if (lhs.m[y][x] != rhs.m[y][x]) {
					return false;
				}
			}
		}

		return true;
	}

	bool operator!=(const Matrix2x2& lhs, const Matrix2x2& rhs) {
		return !(lhs == rhs);
	}
}