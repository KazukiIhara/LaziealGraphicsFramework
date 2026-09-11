#include <LGF/LGF.h>

using namespace LGF;

void Main() {
	Model duckModel("Assets/Models/Duck/Duck.glb");
	Transform3D modelTrans{};
	modelTrans.translation = { 0.0f, -0.1f, 0.0f };
	const Matrix4x4 modelWorldMatrix = Math::MakeAffineMatrix(
		modelTrans.scale,
		modelTrans.rotation,
		modelTrans.translation);

	DebugCamera3D camera(5.0f);
	DirectionalLight directionalLight({ 1.0f, -1.0f, 0.5f }, Color::White, 1.5f);

	while (System::Update()) {
		camera.Update();
		camera.SetScene();
		directionalLight.SetScene();

		Grid3D().Draw();
		duckModel.Draw(modelWorldMatrix);
	}
}
