#include <LGF/LGF.h>

using namespace LGF;

void Main() {
	Model duckModel("Assets/Models/Duck/Duck.glb");
	Transform3D modelTrans{};
	// 固定したカリング視錐台の右端に置き、一部のメッシュレットが外れるようにする。
	modelTrans.translation = { 2.5f, -0.1f, 0.0f };
	constexpr float kModelMoveSpeed = 2.0f;

	DebugCamera3D camera(7.0f);
	Camera3D::Param cullingCameraParam{};
	cullingCameraParam.farClip = 10.0f;
	Camera3D cullingCamera(
		{ 0.0f, 1.3f, -5.0f },
		0.0f,
		15.0f * Math::DEG_TO_RAD,
		cullingCameraParam);
	Camera3DDebugDraw cullingCameraDebugDraw{
		.showFrustum = true,
		.frustumColor = Color::Yellow,
	};
	const ModelDebugDraw modelDebugDraw{
		.showMeshletColors = true,
		.showMeshletBoundingSpheres = true,
		.meshletBoundingSphereColor = Color::Lime,
	};
	DirectionalLight directionalLight({ 1.0f, -1.0f, 0.5f }, Color::White, 1.5f);

	while (System::Update()) {
		Vector3 moveDirection{
			static_cast<float>(Key::D.Press()) - static_cast<float>(Key::A.Press()),
			0.0f,
			static_cast<float>(Key::W.Press()) - static_cast<float>(Key::S.Press()),
		};
		if (Math::LengthSq(moveDirection) > Math::EPSILON) {
			moveDirection = Math::Normalize(moveDirection);
			modelTrans.translation +=
				moveDirection * kModelMoveSpeed * static_cast<float>(System::DeltaTime());
		}

		const Matrix4x4 modelWorldMatrix = Math::MakeAffineMatrix(
			modelTrans.scale,
			modelTrans.rotation,
			modelTrans.translation);

		camera.Update();
		camera.SetScene();
		cullingCamera.SetCullingScene();
		cullingCamera.DrawDebug(cullingCameraDebugDraw);
		directionalLight.SetScene();

		Grid3D().Draw();
		duckModel.Draw(modelWorldMatrix, Color::White, modelDebugDraw);
	}
}
