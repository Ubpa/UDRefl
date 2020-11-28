#include <UDRefl/UDRefl.h>

#include <iostream>
#include <array>

using namespace Ubpa::UDRefl;

struct A { float a{ 0.f }; };
struct B : virtual A { float b{ 0.f }; };
struct C : virtual A { float c{ 0.f }; };
struct D : B, C { float d{ 0.f }; };

int main() {
	auto ID_A = ReflMngr::Instance().tregistry.Register("A");
	auto ID_B = ReflMngr::Instance().tregistry.Register("B");
	auto ID_C = ReflMngr::Instance().tregistry.Register("C");
	auto ID_D = ReflMngr::Instance().tregistry.Register("D");
	auto ID_float = ReflMngr::Instance().tregistry.Register("float");

	auto ID_a = ReflMngr::Instance().nregistry.Register("a");
	auto ID_b = ReflMngr::Instance().nregistry.Register("b");
	auto ID_c = ReflMngr::Instance().nregistry.Register("c");
	auto ID_d = ReflMngr::Instance().nregistry.Register("d");

	{ // register
		TypeInfo typeinfo_A{
			{ // fieldinfos
				{ ID_a, {{ID_float, offsetof(A,a) }}}
			}
		};
		TypeInfo typeinfo_B{
			{ // fieldinfos
				{ ID_b, {{ID_float, field_offset_function<&B::b>() }}}
			},
			{}, // methodinfos
			{ // baseinfos
				{ID_A, BaseInfo::Make<B,A>()}
			},
		};
		TypeInfo typeinfo_C{
			{ // fieldinfos
				{ ID_c, {{ID_float, field_offset_function<&C::c>() }}}
			},
			{}, // methodinfos
			{ // baseinfos
				{ID_A, BaseInfo::Make<C,A>()}
			},
		};
		TypeInfo typeinfo_D{
			{ // fieldinfos
				{ ID_d, {{ID_float, field_offset_function<&D::d>() }}}
			},
			{}, // methodinfos
			{ //baseinfos
				{ID_B, BaseInfo::Make<D,B>()},
				{ID_C, BaseInfo::Make<D,C>()},
			},
		};

		ReflMngr::Instance().typeinfos.emplace(ID_A, std::move(typeinfo_A));
		ReflMngr::Instance().typeinfos.emplace(ID_B, std::move(typeinfo_B));
		ReflMngr::Instance().typeinfos.emplace(ID_C, std::move(typeinfo_C));
		ReflMngr::Instance().typeinfos.emplace(ID_D, std::move(typeinfo_D));
	}

	D d;
	d.a = 1.f;
	d.b = 2.f;
	d.c = 3.f;
	d.d = 4.f;

	ObjectPtr ptr{ ID_D, &d };
	
	ReflMngr::Instance().RWField(ptr, ID_a).As<float>() = 10.f;

	ReflMngr::Instance().ForEachRField(
		ptr,
		[](TypeFieldInfo info, ConstObjectPtr field) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(info.fieldID)
				<< ": " << field.As<float>()
				<< std::endl;
		}
	);

	return 0;
}
