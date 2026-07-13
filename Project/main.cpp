#include <LGF/LGF.h>

using namespace LGF;

void Main() {

	Transform3D trans{};
	while (System::Update()) {

		Camera3D(Vector3(1.0f, 0.0f, -5.0f)).SetScene();

		Triangle3D().Draw(Math::MakeAffineMatrix(trans.scale, trans.rotation, trans.translation), Color::Violet);

		Triangle2D().Draw(Color::Red);
		Rect2D({ 150.0f, 50.0f }, { 100.0f, 100.0f }).Draw(Color::Blue);
		Circle2D({ 350.0f, 100.0f }, 50.0f).Draw(Color::Green);
		Line2D({ 100.0f, 220.0f }, { 400.0f, 260.0f }, 1.0f).Draw(Color::White);
		RoundRect2D({ 450.0f, 50.0f }, { 180.0f, 100.0f }, 24.0f).Draw(Color::White);
	}

}
