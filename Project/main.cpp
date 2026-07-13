#include <LGF/LGF.h>

using namespace LGF;

void Main() {

	Transform3D trans{};
	trans.rotation = Math::MakeQuaternion(Vector3(0.0f, Math::PI, 0.0f));
	while (System::Update()) {

		Camera3D(Vector3(0.0f, 1.0f, -5.0f)).SetScene();
		Matrix4x4 worldMatrix = Math::MakeAffineMatrix(trans.scale, trans.rotation, trans.translation);

		Triangle3D().Draw(worldMatrix, Color::Violet);
		Plane3D().Draw(worldMatrix, Color::White);

		Triangle2D({ 50.0f,50.0f }, { 100.0f,150.0f }, { 0.0f,150.0f }).Draw(Color::Red);
		Rect2D({ 150.0f, 50.0f }, { 100.0f, 100.0f }).Draw(Color::Blue);
		Circle2D({ 350.0f, 100.0f }, 50.0f).Draw(Color::Green);
		Line2D({ 100.0f, 220.0f }, { 400.0f, 260.0f }, 1.0f).Draw(Color::White);
		RoundRect2D({ 450.0f, 50.0f }, { 180.0f, 100.0f }, 24.0f).Draw(Color::White);
	}

}
