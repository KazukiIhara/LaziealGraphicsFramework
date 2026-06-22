#include <LGF/LGF.h>

using namespace LGF;

void Main() {

	Transform3D trans = Transform3D(Vector3(0.0f, 0.0f, 5.0f));

	while (System::Update()) {
		trans.Update();
		Triangle3D{}.Draw(trans.GetWorldMatrix());
	}

}