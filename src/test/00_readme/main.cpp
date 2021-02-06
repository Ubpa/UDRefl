#include <UDRefl/UDRefl.h>
#include <iostream>
#include <cmath>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Vec {
	float x;
	float y;
	float norm() const {
		return std::sqrt(x * x + y * y);
	}
};

int main() {
	Mngr.RegisterType<Vec>();
	Mngr.AddField<&Vec::x>("x");
	Mngr.AddField<&Vec::y>("y");
	Mngr.AddMethod<&Vec::norm>("norm");

	SharedObject v = Mngr.MakeShared(Type_of<Vec>);
	std::cout << v.GetType().GetName() << std::endl; // prints "Vec"

	v.Var("x") = 3;
	v.Var("y") = 4;

	std::cout << "x: " << v.Var("x") << std::endl;
	std::cout << "norm: " << v.DMInvoke("norm") << std::endl;

	for (auto&& field : Mngr.GetFields(Type_of<Vec>))
		std::cout << field.name.GetView() << std::endl;

	for (auto&& method : Mngr.GetMethods(Type_of<Vec>))
		std::cout << method.name.GetView() << std::endl;

	for (auto&& [type, field, var] : v.GetTypeFieldVars())
		std::cout << field.name.GetView() << ": " << var << std::endl;
}
