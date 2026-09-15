#include <LGF/LGF.h>

using namespace LGF;

void Main() {
	Model model("Assets/Models/FlightHelmet/FlightHelmet.gltf");
	Transform3D modelTrans{};
	modelTrans.scale = { 5.0f, 5.0f, 5.0f };
	modelTrans.translation = { 0.0f, -1.75f, 0.0f };
	constexpr float kModelMoveSpeed = 2.0f;
	const ModelDrawOptions modelDrawOptions{
		.enableNormalMap = true,
		.normalScale = 1.0f,
	};

	DebugCamera3D camera(10.0f);
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

		camera.Update();
		camera.SetScene();
		directionalLight.SetScene();

		Grid3D().Draw();
		model.Draw(Math::MakeAffineMatrix(
			modelTrans.scale,
			modelTrans.rotation,
			modelTrans.translation),
			modelDrawOptions);
	}
}
