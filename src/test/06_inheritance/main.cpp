#include <UDRefl/UDRefl.h>

#include <iostream>
#include <array>

using namespace Ubpa::UDRefl;

struct A { float a; };
struct B : A { float b; };
struct C : A { float c; };
struct D : B, C { float d; };

int main() {
	size_t ID_A = ReflMngr::Instance().registry.Register("A");
	size_t ID_B = ReflMngr::Instance().registry.Register("B");
	size_t ID_C = ReflMngr::Instance().registry.Register("C");
	size_t ID_D = ReflMngr::Instance().registry.Register("D");
	size_t ID_a = ReflMngr::Instance().registry.Register("a");
	size_t ID_b = ReflMngr::Instance().registry.Register("b");
	size_t ID_c = ReflMngr::Instance().registry.Register("c");
	size_t ID_d = ReflMngr::Instance().registry.Register("d");
	size_t ID_float = ReflMngr::Instance().registry.Register("float");

	{ // register
		TypeInfo typeinfo_A{
			{ // fieldinfos
				{ ID_a, {{ID_float, offsetof(A,a) }}}
			}
		};
		TypeInfo typeinfo_B{
			{ // fieldinfos
				{ ID_b, {{ID_float, offsetof(B,b) }}}
			},
			{}, // methodinfos
			{ //baseinfos
				{ID_A, {inherit_cast_functions<B, A>()}}
			}
		};
		TypeInfo typeinfo_C{
			{ // fieldinfos
				{ ID_c, {{ID_float, offsetof(C,c) }}}
			},
			{}, // methodinfos
			{ //baseinfos
				{ID_A, {inherit_cast_functions<C, A>()}}
			}
		};
		TypeInfo typeinfo_D{
			{ // fieldinfos
				{ ID_d, {{ID_float, offsetof(D,d) }}}
			},
			{}, // methodinfos
			{ //baseinfos
				{ID_B, {inherit_cast_functions<D, B>()}},
				{ID_C, {inherit_cast_functions<D, C>()}},
			}
		};

		ReflMngr::Instance().typeinfos.emplace(ID_A, std::move(typeinfo_A));
		ReflMngr::Instance().typeinfos.emplace(ID_B, std::move(typeinfo_B));
		ReflMngr::Instance().typeinfos.emplace(ID_C, std::move(typeinfo_C));
		ReflMngr::Instance().typeinfos.emplace(ID_D, std::move(typeinfo_D));
	}

	D d;
	d.C::a = 1.f;
	d.B::a = 2.f;
	d.b = 3.f;
	d.c = 4.f;
	d.d = 5.f;

	ObjectPtr ptr{ ID_D, &d };
	
	ReflMngr::Instance().RWField(ReflMngr::Instance().StaticCast_DerivedToBase(ptr, ID_C), ID_a).As<float>() = 10.f;

	ReflMngr::Instance().ForEachRField(
		ptr,
		[](size_t typeID, const TypeInfo& typeinfo, size_t fieldID, const FieldInfo& fieldinfo, ConstObjectPtr field) {
			std::cout
				<< ReflMngr::Instance().registry.Nameof(fieldID)
				<< ": " << field.As<float>()
				<< std::endl;
		}
	);

	return 0;
}
