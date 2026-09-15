#include <LGF/LGF.h>

using namespace LGF;

void Main() {
	Model flightHelmet("Assets/Models/FlightHelmet/FlightHelmet.gltf");
	Transform3D flightHelmetTransform{};
	flightHelmetTransform.scale = { 5.0f, 5.0f, 5.0f };
	flightHelmetTransform.translation = { -1.5f, -1.75f, 0.0f };

	Model waterBottle("Assets/Models/WaterBottle/WaterBottle.glb");
	Transform3D waterBottleTransform{};
	waterBottleTransform.scale = { 10.0f, 10.0f, 10.0f };
	waterBottleTransform.translation = { 1.5f, -0.45f, 0.0f };

	const Ground3D ground(14.0f);
	const Matrix4x4 groundWorldMatrix =
		Math::MakeTranslateMatrix({ 0.0f, -1.75f, 0.0f });
	const Material3D groundMaterial{
		.color = Color::Gray,
	};

	constexpr float kModelMoveSpeed = 2.0f;
	const ModelDrawOptions modelDrawOptions{
		.enableNormalMap = true,
		.normalScale = 1.0f,
	};

	DebugCamera3D camera(10.0f);
	DirectionalLight directionalLight({ 1.0f, -1.0f, 0.5f }, Color::White, 1.5f);
	Skybox skybox("Assets/Textures/Environment/StoryStudio02/story_studio_02_2k.dds");

	while (System::Update()) {
		Vector3 moveDirection{
			static_cast<float>(Key::D.Press()) - static_cast<float>(Key::A.Press()),
			0.0f,
			static_cast<float>(Key::W.Press()) - static_cast<float>(Key::S.Press()),
		};
		if (Math::LengthSq(moveDirection) > Math::EPSILON) {
			moveDirection = Math::Normalize(moveDirection);
			flightHelmetTransform.translation +=
				moveDirection * kModelMoveSpeed * static_cast<float>(System::DeltaTime());
		}

		camera.Update();
		camera.SetScene();
		directionalLight.SetScene();
		skybox.SetScene();

		ground.Draw(groundWorldMatrix, groundMaterial);
		flightHelmet.Draw(Math::MakeAffineMatrix(
			flightHelmetTransform.scale,
			flightHelmetTransform.rotation,
			flightHelmetTransform.translation),
			modelDrawOptions);
		waterBottle.Draw(Math::MakeAffineMatrix(
			waterBottleTransform.scale,
			waterBottleTransform.rotation,
			waterBottleTransform.translation),
			modelDrawOptions);
	}
}
