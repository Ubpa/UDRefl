#include <UDRefl/UDRefl.hpp>
#include <UDRefl_ext/Bootstrap.h>
#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct A {
	float data;
};

int main() {
	Mngr.RegisterType<A>();
	Mngr.AddField<&A::data>("data");
	Ubpa_UDRefl_ext_Bootstrap();
	A a;
	ObjectView{ a }.Var("data") = 3;
	std::cout << a.data << std::endl;
	auto data = MngrView.Invoke<ObjectView>("Var", TempArgsView{ a, Name{ "data" } });
	data = 4;
	std::cout << a.data << std::endl;

	return 0;
}
