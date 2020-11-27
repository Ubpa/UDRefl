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
			{}, // attrs
			{
				{ ID_a, {{ID_float, offsetof(A,a) }}}
			} // fieldinfos
		};
		TypeInfo typeinfo_B{
			{}, // attrs
			{
				{ ID_b, {{ID_float, offsetof(B,b) }}}
			}, // fieldinfos
			{}, // methodinfos
			{
				{ID_A, {base_offset<B, A>()}}
			}//baseinfos
		};
		TypeInfo typeinfo_C{
			{}, // attrs
			{
				{ ID_c, {{ID_float, offsetof(C,c) }}}
			}, // fieldinfos
			{}, // methodinfos
			{
				{ID_A, {base_offset<C, A>()}}
			}//baseinfos
		};
		TypeInfo typeinfo_D{
			{}, // attrs
			{
				{ ID_d, {{ID_float, offsetof(D,d) }}}
			}, // fieldinfos
			{}, // methodinfos
			{
				{ID_B, {base_offset<D, B>()}},
				{ID_C, {base_offset<D, C>()}},
			}//baseinfos
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
	
	ReflMngr::Instance().RWField(ReflMngr::Instance().Cast(ptr, ID_C), ID_a).As<float>() = 10.f;

	std::cout << ReflMngr::Instance().RField(ReflMngr::Instance().Cast(ptr, ID_B), ID_a).As<float>() << std::endl;
	std::cout << ReflMngr::Instance().RField(ReflMngr::Instance().Cast(ptr, ID_C), ID_a).As<float>() << std::endl;
	std::cout << ReflMngr::Instance().RField(ptr, ID_b).As<float>() << std::endl;
	std::cout << ReflMngr::Instance().RField(ptr, ID_c).As<float>() << std::endl;
	std::cout << ReflMngr::Instance().RField(ptr, ID_d).As<float>() << std::endl;

	return 0;
}
