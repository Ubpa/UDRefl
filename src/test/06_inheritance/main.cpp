#include <UDRefl/UDRefl.h>

#include <iostream>
#include <array>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct A { float a; };
struct B : A { float b; };
struct C : A { float c; };
struct D : B, C { float d; };

int main() {
	{ // register
		Mngr->RegisterType<A>();
		Mngr->AddField<&A::a>("a");

		Mngr->RegisterType<B>();
		Mngr->AddBases<B, A>();
		Mngr->AddField<&B::b>("b");

		Mngr->RegisterType<C>();
		Mngr->AddBases<C, A>();
		Mngr->AddField<&C::c>("c");

		Mngr->RegisterType<D>();
		Mngr->AddBases<D, B, C>();
		Mngr->AddField<&D::d>("d");
	}

	auto d = Mngr->MakeShared(Type_of<D>);
	d.Var(Type_of<B>, "a") = 1.f;
	d.Var(Type_of<C>, "a") = 2.f;
	d.Var("b") = 3.f;
	d.Var("c") = 4.f;
	d.Var("d") = 5.f;

	for (const auto& [type, field, var] : d.GetTypeFieldVars()) {
		std::cout
			<< field.name.GetView()
			<< ": " << var
			<< std::endl;
	}

	return 0;
}
