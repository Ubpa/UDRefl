#include <UDRefl/UDRefl.hpp>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct A { float a{ 0.f }; };
struct B : virtual A { float b{ 0.f }; };
struct C : virtual A { float c{ 0.f }; };
struct D : B, C { float d{ 0.f }; };

int main() {
	{ // register
		Mngr.RegisterType<A>();
		Mngr.AddField<&A::a>("a");

		Mngr.RegisterType<B>();
		Mngr.AddBases<B, A>();
		Mngr.AddField<&B::b>("b");

		Mngr.RegisterType<C>();
		Mngr.AddBases<C, A>();
		Mngr.AddField<&C::c>("c");

		Mngr.RegisterType<D>();
		Mngr.AddBases<D, B, C>();
		Mngr.AddField<&D::d>("d");
	}

	auto d = Mngr.MakeShared(Type_of<D>);

	d.Var("a") = 1.f;
	d.Var("b") = 3.f;
	d.Var("c") = 4.f;
	d.Var("d") = 5.f;

	for (const auto& [name, fieldinfo] : FieldRange_of<D>)
		std::cout << name.GetView() << std::endl;
	for (const auto& [name, var] : d.GetVars())
		std::cout << name.GetView() << ": " << var << std::endl;

	return 0;
}
