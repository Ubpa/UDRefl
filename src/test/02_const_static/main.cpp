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
	Mngr.AddField(TypeID_of<A>, "s_data", &A::s_data);
	Mngr.AddField(TypeID_of<A>, "sc_data", &A::sc_data);

	A a;
	ObjectView ptr{ a };

	for (auto field : Mngr.GetFields(TypeID_of<A>))
		std::cout << Mngr.nregistry.Nameof(field.ID) << std::endl;

	ptr.Var("data") = 10;
	ptr.Var("s_data") = 20;

	std::cout
		<< "//" << std::endl
		<< "// unowned var" << std::endl
		<< "////////////////" << std::endl;
	for (auto [type, field, var] : Mngr.GetTypeFieldVars(TypeID_of<A>))
		std::cout << Mngr.nregistry.Nameof(field.ID) << ": " << var << std::endl;

	std::cout
		<< "//" << std::endl
		<< "// object var" << std::endl
		<< "///////////////" << std::endl;
	for (auto [type, field, var] : ptr.GetTypeFieldVars())
		std::cout << Mngr.nregistry.Nameof(field.ID) << ": " << var << std::endl;

	std::cout
		<< "//" << std::endl
		<< "// object owned var" << std::endl
		<< "/////////////////////" << std::endl;
	for (auto [type, field, var] : ptr.GetTypeFieldOwnedVars())
		std::cout << Mngr.nregistry.Nameof(field.ID) << ": " << var << std::endl;

	return 0;
}
