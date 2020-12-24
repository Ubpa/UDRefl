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
		ReflMngr::Instance().RegisterTypeAuto<A>();
		ReflMngr::Instance().AddField<&A::a>("a");

		ReflMngr::Instance().RegisterTypeAuto<B>();
		ReflMngr::Instance().AddBases<B, A>();
		ReflMngr::Instance().AddField<&B::b>("b");

		ReflMngr::Instance().RegisterTypeAuto<C>();
		ReflMngr::Instance().AddBases<C, A>();
		ReflMngr::Instance().AddField<&C::c>("c");

		ReflMngr::Instance().RegisterTypeAuto<D>();
		ReflMngr::Instance().AddBases<D, B, C>();
		ReflMngr::Instance().AddField<&D::d>("d");
	}

	auto d = ReflMngr::Instance().MakeShared(TypeID_of<D>);

	ReflMngr::Instance().RWVar(d, "a") = 1.f;
	ReflMngr::Instance().RWVar(d, "b") = 2.f;
	ReflMngr::Instance().RWVar(d, "c") = 3.f;
	ReflMngr::Instance().RWVar(d, "d") = 4.f;

	d->ForEachRVar(
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var
				<< std::endl;
			return true;
		}
	);

	return 0;
}
