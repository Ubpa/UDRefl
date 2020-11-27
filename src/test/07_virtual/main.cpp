#include <UDRefl/UDRefl.h>

#include <iostream>
#include <array>

using namespace Ubpa::UDRefl;

struct O { float o{ 0.f }; };
struct A { float a{ 0.f }; };
struct B : O, virtual A { inline static int x; float b{ 0.f }; };
struct C : virtual A { float c{ 0.f }; };
struct D : B, C { float d{ 0.f }; };

int main() {
	has_virtual_base_v<A>;
	has_virtual_base_v<B>;

	size_t offset = 1;
	bool a;
	std::cin >> a;
	const void* (*offset_functor)(const void*);
	if (a)
		offset_functor = [](const void* ptr) {
			return forward_offset(ptr, 1);
		};
	else
		offset_functor = [](const void* ptr) {
			return forward_offset(ptr, 2);
		};
	const void* t = nullptr;
	std::cout << forward_offset(t, 1) << std::endl;
	std::cout << offset_functor(t) << std::endl;

	size_t ID_A = ReflMngr::Instance().registry.Register("A");
	size_t ID_B = ReflMngr::Instance().registry.Register("B");
	size_t ID_C = ReflMngr::Instance().registry.Register("C");
	size_t ID_D = ReflMngr::Instance().registry.Register("D");
	size_t ID_a = ReflMngr::Instance().registry.Register("a");
	size_t ID_b = ReflMngr::Instance().registry.Register("b");
	size_t ID_c = ReflMngr::Instance().registry.Register("c");
	size_t ID_d = ReflMngr::Instance().registry.Register("d");
	size_t ID_float = ReflMngr::Instance().registry.Register("float");
	//reinterpret_cast<const void*>(&B::b);
	field_offset_functor<&B::b>();
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
			{}, // cmethodinfos
			{}, // smethodinfos
			{}, // baseinfos
			{
				{ID_A, {base_offset_functor<B, A>()}}
			}//vbaseinfos
		};
		TypeInfo typeinfo_C{
			{}, // attrs
			{
				{ ID_c, {{ID_float, offsetof(C,c) }}}
			}, // fieldinfos
			{}, // methodinfos
			{}, // cmethodinfos
			{}, // smethodinfos
			{}, // baseinfos
			{
				{ID_A, {base_offset_functor<C, A>()}}
			}//vbaseinfos
		};
		TypeInfo typeinfo_D{
			{}, // attrs
			{
				{ ID_d, {{ID_float, offsetof(D,d) }}}
			}, // fieldinfos
			{}, // methodinfos
			{}, // cmethodinfos
			{}, // smethodinfos
			{
				{ID_B, {base_offset<D, B>()}},
				{ID_C, {base_offset<D, C>()}},
			}, //baseinfos
		};

		ReflMngr::Instance().typeinfos.emplace(ID_A, std::move(typeinfo_A));
		ReflMngr::Instance().typeinfos.emplace(ID_B, std::move(typeinfo_B));
		ReflMngr::Instance().typeinfos.emplace(ID_C, std::move(typeinfo_C));
		ReflMngr::Instance().typeinfos.emplace(ID_D, std::move(typeinfo_D));
	}

	D d;
	//d.a = 1.f;
	d.b = 2.f;
	d.c = 3.f;
	d.d = 4.f;

	ObjectPtr ptr{ ID_D, &d };
	
	ReflMngr::Instance().RWField(ptr, ID_a).As<float>() = 10.f;
	std::cout << ReflMngr::Instance().RField(ptr, ID_a).As<float>() << std::endl;
	std::cout << ReflMngr::Instance().RField(ptr, ID_b).As<float>() << std::endl;
	std::cout << ReflMngr::Instance().RField(ptr, ID_c).As<float>() << std::endl;
	std::cout << ReflMngr::Instance().RField(ptr, ID_d).As<float>() << std::endl;

	return 0;
}
