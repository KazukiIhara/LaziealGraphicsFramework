#include <LGF/LGF.h>

using namespace LGF;

void Main() {

	Transform3D trans{};
	while (System::Update()) {

		Camera3D(Vector3(1.0f, 0.0f, -5.0f)).SetScene();

		//Triangle3D().Draw(Math::MakeAffineMatrix(trans.scale, trans.rotation, trans.translation), Color::Violet);

		Triangle2D().Draw(Color::Red);
	}

}
