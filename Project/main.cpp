#include <LGF/LGF.h>

using namespace LGF;

void Main() {

	Transform3D trans{};
	Transform2D trans2D{};

	while (System::Update()) {

		Camera3D(Vector3(1.0f, 0.0f, -5.0f)).SetScene();

		//Triangle3D().Draw(Math::MakeAffineMatrix(trans.scale, trans.rotation, trans.translation), Color::Violet);

		Triangle2D().Draw(Math::MakeAffineMatrix(trans2D.scale, trans2D.rotation, trans2D.translation), Color::Red);
	}

}