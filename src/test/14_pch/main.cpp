#include <UDRefl/UDRefl.h>
#include <iostream>

#include "A.h"
#include "B.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

int main() {
	RegisterA();
	RegisterB();

	for (const auto& [ID, info] : Mngr.typeinfos) {
		for (auto method : Mngr.GetMethods(ID))
			std::cout << Mngr.nregistry.Nameof(method.ID) << std::endl;
	}

	return 0;
}
