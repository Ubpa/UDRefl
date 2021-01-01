#include <UDRefl/UDRefl.h>
#include <iostream>

#include "A.h"
#include "B.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

int main() {
	RegisterA();
	RegisterB();

	for (const auto& [ID, info] : ReflMngr::Instance().typeinfos) {
		for (auto method : ReflMngr::Instance().GetMethods(ID))
			std::cout << ReflMngr::Instance().nregistry.Nameof(method.ID) << std::endl;
	}

	return 0;
}
