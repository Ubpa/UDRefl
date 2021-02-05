#include <UDRefl/UDRefl.h>
#include <iostream>
#include <cmath>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Vec {
	float x;
	float y;
	float norm() const noexcept {
		return std::sqrt(x * x + y * y);
	}
};

int main() {
	Mngr.RegisterType<Vec>();
	Mngr.AddField<&Vec::x>("x");
	Mngr.AddField<&Vec::y>("y");
	Mngr.AddMethod<&Vec::norm>("norm");

	SharedObject v = Mngr.MakeShared(TypeID_of<Vec>);
	std::cout << v.TypeName() << std::endl; // prints "Vec"

	v.Var("x") = 3.f;
	v.Var("y") = 4.f;

	std::cout << "x: " << v.Var("x") << std::endl;
	std::cout << "norm: " << v.DMInvoke("norm") << std::endl;
	
	for (auto field : Mngr.GetFields(TypeID_of<Vec>))
		std::cout << Mngr.nregistry.Nameof(field.ID) << std::endl;

	for (auto method : Mngr.GetMethods(TypeID_of<Vec>))
		std::cout << Mngr.nregistry.Nameof(method.ID) << std::endl;

	for (const auto& [type, field, var] : v.GetTypeFieldVars()) {
		std::cout
			<< Mngr.nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}
}
