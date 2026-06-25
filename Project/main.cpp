#include <LGF/LGF.h>

using namespace LGF;

void Main() {

	Transform3D trans{};
	trans.translation.z = 5;

	while (System::Update()) {

		Triangle3D{}.Draw(Math::MakeAffineMatrix(trans.scale, trans.rotation, trans.translation), Color::Violet);
	}

}