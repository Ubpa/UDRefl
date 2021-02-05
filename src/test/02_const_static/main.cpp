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
	ReflMngr::Instance().RegisterType<A>();
	ReflMngr::Instance().AddField<&A::data>("data");
	ReflMngr::Instance().AddField<&A::c_data>("c_data");
	ReflMngr::Instance().AddField(TypeID_of<A>, "s_data", &A::s_data);
	ReflMngr::Instance().AddField(TypeID_of<A>, "sc_data", &A::sc_data);

	A a;
	ObjectPtr ptr{ a };

	for (auto field : ReflMngr::Instance().GetFields(TypeID_of<A>))
		std::cout << ReflMngr::Instance().nregistry.Nameof(field.ID) << std::endl;

	ReflMngr::Instance().Var(ptr, "data") = 10;
	ReflMngr::Instance().Var(ptr, "s_data") = 20;

	std::cout
		<< "//" << std::endl
		<< "// unowned var" << std::endl
		<< "////////////////" << std::endl;
	for (auto [type, field, var] : ReflMngr::Instance().GetTypeFieldVars(TypeID_of<A>))
		std::cout << ReflMngr::Instance().nregistry.Nameof(field.ID) << ": " << var << std::endl;

	std::cout
		<< "//" << std::endl
		<< "// object var" << std::endl
		<< "///////////////" << std::endl;
	for (auto [type, field, var] : ptr->GetTypeFieldVars())
		std::cout << ReflMngr::Instance().nregistry.Nameof(field.ID) << ": " << var << std::endl;

	std::cout
		<< "//" << std::endl
		<< "// object owned var" << std::endl
		<< "/////////////////////" << std::endl;
	for (auto [type, field, var] : ptr->GetTypeFieldOwnedVars())
		std::cout << ReflMngr::Instance().nregistry.Nameof(field.ID) << ": " << var << std::endl;

	return 0;
}
