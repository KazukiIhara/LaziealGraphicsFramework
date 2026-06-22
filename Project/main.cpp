#include <LGF/LGF.h>

using namespace LGF;

void Main() {

	while (LGF::System::Update()) {
		Transform3D trans = Transform3D(Vector3(0.0f, 0.0f, 5.0f));
		trans.Update();
		Triangle3D().Draw(trans);
	}

}