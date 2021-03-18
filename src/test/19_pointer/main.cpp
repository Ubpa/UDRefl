#include <UDRefl/UDRefl.hpp>
#include <iostream>
#include <cmath>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

int main() {
	Mngr.RegisterType<int**>();
	int i = 0;
	int* pi = &i;
	int** ppi = &pi;

	std::cout
		<< "//" << std::endl
		<< "// int**" << std::endl
		<< "//////////" << std::endl;
	for (auto&& [name, info] : MethodRange_of<int**>)
		std::cout << name.GetView() << std::endl;
	std::cout
		<< "//" << std::endl
		<< "// int*" << std::endl
		<< "/////////" << std::endl;
	for (auto&& [name, info] : MethodRange_of<int*>)
		std::cout << name.GetView() << std::endl;

	SharedObject ref_i = **ObjectView{ ppi };
	ref_i = 3;

	std::cout << i << std::endl;

	return 0;
}
