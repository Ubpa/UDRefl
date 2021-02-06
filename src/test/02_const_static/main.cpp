#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct A {
	int data{ 1 };
	const int c_data{ 2 };
	inline static int s_data{ 3 };
	inline static const int sc_data{ 4 };
};

int main() {
	Mngr.RegisterType<A>();
	Mngr.AddField<&A::data>("data");
	Mngr.AddField<&A::c_data>("c_data");
	Mngr.AddField(Type_of<A>, "s_data", &A::s_data);
	Mngr.AddField(Type_of<A>, "sc_data", &A::sc_data);

	A a;
	ObjectView ptr{ a };

	for (const auto& field : Mngr.GetFields(Type_of<A>))
		std::cout << field.name.GetView() << std::endl;

	ptr.Var("data") = 10;
	ptr.Var("s_data") = 20;

	std::cout
		<< "//" << std::endl
		<< "// unowned var" << std::endl
		<< "////////////////" << std::endl;
	for (const auto& [type, field, var] : Mngr.GetTypeFieldVars(Type_of<A>))
		std::cout << field.name.GetView() << ": " << var << std::endl;

	std::cout
		<< "//" << std::endl
		<< "// object var" << std::endl
		<< "///////////////" << std::endl;
	for (const auto& [type, field, var] : ptr.GetTypeFieldVars())
		std::cout << field.name.GetView() << ": " << var << std::endl;

	std::cout
		<< "//" << std::endl
		<< "// object owned var" << std::endl
		<< "/////////////////////" << std::endl;
	for (const auto& [type, field, var] : ptr.GetTypeFieldOwnedVars())
		std::cout << field.name.GetView() << ": " << var << std::endl;

	return 0;
}
