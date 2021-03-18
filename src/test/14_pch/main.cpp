#include <UDRefl/UDRefl.hpp>
#include <iostream>

#include "A.hpp"
#include "B.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

int main() {
	RegisterA();
	RegisterB();

	for (const auto& [type, info] : Mngr.typeinfos) {
		for (const auto& [name, method] : MethodRange{ type })
			std::cout << name.GetView() << std::endl;
	}

	return 0;
}
