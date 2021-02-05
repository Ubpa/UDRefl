#include <UDRefl/UDRefl.h>
#include <iostream>
#include <tuple>

using namespace Ubpa;
using namespace Ubpa::UDRefl;


int main() {
	Mngr.RegisterType<std::tuple<int, float, double>>();

	SharedObject v = Mngr.MakeShared(TypeID_of<std::tuple<int, float, double>>);
	std::cout << v.TypeName() << std::endl;

	v.Var("__0") = 1;
	v.Var("__1") = 2;
	v.Var("__2") = 3;

	std::cout << "__0: " << v.Var("__0") << std::endl;
	std::cout << "__1: " << v.tuple_get(1) << std::endl;
	std::cout << "__2: " << v.tuple_get(2) << std::endl;
	
	for (auto field : Mngr.GetFields(TypeID_of<std::tuple<int, float, double>>))
		std::cout << Mngr.nregistry.Nameof(field.ID) << std::endl;

	for (auto method : Mngr.GetMethods(TypeID_of<std::tuple<int, float, double>>))
		std::cout << Mngr.nregistry.Nameof(method.ID) << std::endl;

	for (const auto& [type, field, var] : v.GetTypeFieldVars()) {
		std::cout
			<< Mngr.tregistry.Nameof(field.info.fieldptr.GetValueID()) << " "
			<< Mngr.nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}
}
