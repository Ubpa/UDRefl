#include <UDRefl/UDRefl.h>

#include <iostream>
#include <array>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct A { float a{ 0.f }; };
struct B : virtual A { float b{ 0.f }; };
struct C : virtual A { float c{ 0.f }; };
struct D : B, C { float d{ 0.f }; };

int main() {
	{ // register
		ReflMngr::Instance().RegisterType<A>();
		ReflMngr::Instance().AddField<&A::a>("a");

		ReflMngr::Instance().RegisterType<B>();
		ReflMngr::Instance().AddBases<B, A>();
		ReflMngr::Instance().AddField<&B::b>("b");

		ReflMngr::Instance().RegisterType<C>();
		ReflMngr::Instance().AddBases<C, A>();
		ReflMngr::Instance().AddField<&C::c>("c");

		ReflMngr::Instance().RegisterType<D>();
		ReflMngr::Instance().AddBases<D, B, C>();
		ReflMngr::Instance().AddField<&D::d>("d");
	}

	auto d = ReflMngr::Instance().MakeShared(TypeID_of<D>);

	d->Var("a") = 1.f;
	d->Var("b") = 3.f;
	d->Var("c") = 4.f;
	d->Var("d") = 5.f;

	for (const auto& [type, field, var] : d->GetTypeFieldVars()) {
		std::cout
			<< ReflMngr::Instance().nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}

	return 0;
}
