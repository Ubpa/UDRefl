#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct A {
	float data;
};

int main() {
	Mngr.RegisterType<A>();
	Mngr.AddField<&A::data>("data");
	ObjectView mngr = Mngr.ReflSefl();
	A a;
	ObjectView{ a }.Var("data") = 3;
	std::cout << a.data << std::endl;
	auto data = mngr.Invoke<ObjectView>("Var", ObjectView{ a }, Name{ "data" }, FieldFlag::All);
	data = 4;
	std::cout << a.data << std::endl;

	return 0;
}
