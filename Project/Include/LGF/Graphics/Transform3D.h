#pragma once

#include <vector>

#include "LGF/Math/MathTypes.h"

namespace LGF {

	/// <summary>
	/// 3Dトランスフォーム
	/// </summary>
	class Transform3D {
	public:
		Transform3D(
			const Vector3& scale,
			const Quaternion& rotation,
			const Vector3& translation
		);

		explicit Transform3D(
			const Vector3& translation = { 0.0f,0.0f,0.0f }
		);

		~Transform3D();

		void Update();

		void Finalize();

		Matrix4x4 GetWorldMatrix()const;
		Vector3 GetWorldPosition()const;

		void SetParent(Transform3D* parent);

	private:
		Vector3 worldPosition_{};
		Matrix4x4 worldMatrix_{};
		Transform3D* parent_ = nullptr;
		std::vector<Transform3D*> children_;

	public:
		Vector3 scale{ 1.0f,1.0f,1.0f };
		Quaternion rotation{};
		Vector3 translation{};

	};


}