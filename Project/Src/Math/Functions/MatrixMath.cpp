#include "MatrixMath.h"

#include "LGF/Math/MathConstants.h"

#include <cassert>
#include <cmath>
#include <cstdint>

namespace LGF::Math {

	Matrix2x2 Identity2x2() {
		Matrix2x2 result{};

		result.m[0][0] = 1.0f;
		result.m[1][1] = 1.0f;

		return result;
	}

	Matrix3x3 Identity3x3() {
		Matrix3x3 result{};

		result.m[0][0] = 1.0f;
		result.m[1][1] = 1.0f;
		result.m[2][2] = 1.0f;

		return result;
	}

	Matrix4x4 Identity4x4() {
		Matrix4x4 result{};

		result.m[0][0] = 1.0f;
		result.m[1][1] = 1.0f;
		result.m[2][2] = 1.0f;
		result.m[3][3] = 1.0f;

		return result;
	}

	Matrix2x2 Transpose(const Matrix2x2& m) {
		Matrix2x2 result{};

		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				result.m[y][x] = m.m[x][y];
			}
		}

		return result;
	}

	Matrix3x3 Transpose(const Matrix3x3& m) {
		Matrix3x3 result{};

		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				result.m[y][x] = m.m[x][y];
			}
		}

		return result;
	}

	Matrix4x4 Transpose(const Matrix4x4& m) {
		Matrix4x4 result{};

		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				result.m[y][x] = m.m[x][y];
			}
		}

		return result;
	}

	Matrix2x2 Multiply(const Matrix2x2& m1, const Matrix2x2& m2) {
		Matrix2x2 result{};

		for (uint32_t y = 0; y < 2; ++y) {
			for (uint32_t x = 0; x < 2; ++x) {
				for (uint32_t i = 0; i < 2; ++i) {
					result.m[y][x] += m1.m[y][i] * m2.m[i][x];
				}
			}
		}

		return result;
	}

	Matrix3x3 Multiply(const Matrix3x3& m1, const Matrix3x3& m2) {
		Matrix3x3 result{};

		for (uint32_t y = 0; y < 3; ++y) {
			for (uint32_t x = 0; x < 3; ++x) {
				for (uint32_t i = 0; i < 3; ++i) {
					result.m[y][x] += m1.m[y][i] * m2.m[i][x];
				}
			}
		}

		return result;
	}

	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
		Matrix4x4 result{};

		for (uint32_t y = 0; y < 4; ++y) {
			for (uint32_t x = 0; x < 4; ++x) {
				for (uint32_t i = 0; i < 4; ++i) {
					result.m[y][x] += m1.m[y][i] * m2.m[i][x];
				}
			}
		}

		return result;
	}

	float Determinant(const Matrix2x2& m) {
		return m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0];
	}

	float Determinant(const Matrix3x3& m) {
		return
			m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
			m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) +
			m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
	}

	float Determinant(const Matrix4x4& m) {
		float result = 0.0f;

		for (uint32_t x = 0; x < 4; ++x) {
			Matrix3x3 minor{};

			for (uint32_t y = 1; y < 4; ++y) {
				uint32_t minorX = 0;

				for (uint32_t mx = 0; mx < 4; ++mx) {
					if (mx == x) {
						continue;
					}

					minor.m[y - 1][minorX] = m.m[y][mx];
					++minorX;
				}
			}

			if (x % 2 == 0) {
				result += m.m[0][x] * Determinant(minor);
			} else {
				result -= m.m[0][x] * Determinant(minor);
			}
		}

		return result;
	}

	Matrix2x2 Inverse(const Matrix2x2& m) {
		const float det = Determinant(m);
		assert(std::fabs(det) > EPSILON);

		if (std::fabs(det) <= EPSILON) {
			return Identity2x2();
		}

		const float invDet = 1.0f / det;

		Matrix2x2 result{};
		result.m[0][0] = m.m[1][1] * invDet;
		result.m[0][1] = -m.m[0][1] * invDet;
		result.m[1][0] = -m.m[1][0] * invDet;
		result.m[1][1] = m.m[0][0] * invDet;

		return result;
	}

	Matrix3x3 Inverse(const Matrix3x3& m) {
		const float det = Determinant(m);
		assert(std::fabs(det) > EPSILON);

		if (std::fabs(det) <= EPSILON) {
			return Identity3x3();
		}

		const float invDet = 1.0f / det;

		Matrix3x3 result{};

		result.m[0][0] = (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) * invDet;
		result.m[0][1] = -(m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]) * invDet;
		result.m[0][2] = (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]) * invDet;

		result.m[1][0] = -(m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) * invDet;
		result.m[1][1] = (m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0]) * invDet;
		result.m[1][2] = -(m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0]) * invDet;

		result.m[2][0] = (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]) * invDet;
		result.m[2][1] = -(m.m[0][0] * m.m[2][1] - m.m[0][1] * m.m[2][0]) * invDet;
		result.m[2][2] = (m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0]) * invDet;

		return result;
	}

	Matrix4x4 Inverse(const Matrix4x4& m) {
		Matrix4x4 a = m;
		Matrix4x4 result = Identity4x4();

		for (uint32_t i = 0; i < 4; ++i) {
			float pivot = a.m[i][i];
			uint32_t pivotRow = i;

			for (uint32_t y = i + 1; y < 4; ++y) {
				if (std::fabs(a.m[y][i]) > std::fabs(pivot)) {
					pivot = a.m[y][i];
					pivotRow = y;
				}
			}

			assert(std::fabs(pivot) > EPSILON);

			if (std::fabs(pivot) <= EPSILON) {
				return Identity4x4();
			}

			if (pivotRow != i) {
				for (uint32_t x = 0; x < 4; ++x) {
					const float tempA = a.m[i][x];
					a.m[i][x] = a.m[pivotRow][x];
					a.m[pivotRow][x] = tempA;

					const float tempR = result.m[i][x];
					result.m[i][x] = result.m[pivotRow][x];
					result.m[pivotRow][x] = tempR;
				}
			}

			const float invPivot = 1.0f / a.m[i][i];

			for (uint32_t x = 0; x < 4; ++x) {
				a.m[i][x] *= invPivot;
				result.m[i][x] *= invPivot;
			}

			for (uint32_t y = 0; y < 4; ++y) {
				if (y == i) {
					continue;
				}

				const float factor = a.m[y][i];

				for (uint32_t x = 0; x < 4; ++x) {
					a.m[y][x] -= factor * a.m[i][x];
					result.m[y][x] -= factor * result.m[i][x];
				}
			}
		}

		return result;
	}

	Vector2 Transform(const Vector2& v, const Matrix3x3& m) {
		Vector2 result{};

		result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + m.m[2][0];
		result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + m.m[2][1];

		const float w = v.x * m.m[0][2] + v.y * m.m[1][2] + m.m[2][2];

		if (std::fabs(w) > EPSILON) {
			result.x /= w;
			result.y /= w;
		}

		return result;
	}

	Vector3 Transform(const Vector3& v, const Matrix4x4& m) {
		Vector3 result{};

		result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
		result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
		result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];

		const float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];

		if (std::fabs(w) > EPSILON) {
			result.x /= w;
			result.y /= w;
			result.z /= w;
		}

		return result;
	}

	Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m) {
		Vector3 result{};

		result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0];
		result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1];
		result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2];

		return result;
	}
}