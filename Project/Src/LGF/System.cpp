#include "LGF/System.h"

#include "Framework/Framework.h"

namespace LGF::Detail {
	Framework& GetFramework() {
		static Framework framework;
		return framework;
	}
}

bool LGF::System::Update() {
	return Detail::GetFramework().Update();
}