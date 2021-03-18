#include <UDRefl/UDRefl.hpp>
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
	std::cout << "norm: " << v.Invoke("norm") << std::endl;

	for (auto&& [name, info] : FieldRange_of<Vec>)
		std::cout << name.GetView() << std::endl;

	for (auto&& [name, info] : MethodRange_of<Vec>)
		std::cout << name.GetView() << std::endl;

	for (auto&& [name, var] : v.GetVars())
		std::cout << name.GetView() << ": " << var << std::endl;
}
