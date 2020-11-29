#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa::UDRefl;

struct A {
	int data{ 1 };
	const int c_data{ 2 };
	inline static int s_data{ 3 };
	inline static const int sc_data{ 4 };
};

int main() {
	auto ID_A = ReflMngr::Instance().tregistry.Register("A");
	auto ID_int = ReflMngr::Instance().tregistry.Register("int");
	auto ID_data = ReflMngr::Instance().nregistry.Register("data");
	auto ID_c_data = ReflMngr::Instance().nregistry.Register("c_data");
	auto ID_s_data = ReflMngr::Instance().nregistry.Register("s_data");
	auto ID_sc_data = ReflMngr::Instance().nregistry.Register("sc_data");

	{ // register Point
		FieldPtr ptr_data{ ID_int, offsetof(A, data) };
		FieldPtr ptr_c_data{ ID_int, offsetof(A, c_data), true };
		FieldPtr ptr_s_data{ ID_int, &A::s_data };
		FieldPtr ptr_sc_data{ ID_int, &A::sc_data };

		FieldInfo fieldinfo_data{ ptr_data };
		FieldInfo fieldinfo_c_data{ ptr_c_data };
		FieldInfo fieldinfo_s_data{ ptr_s_data };
		FieldInfo fieldinfo_sc_data{ ptr_sc_data };
		TypeInfo typeinfo{
			sizeof(A),
			alignof(A),
			{ // fieldinfos
				{ID_data, fieldinfo_data},
				{ID_c_data, fieldinfo_c_data},
				{ID_s_data, fieldinfo_s_data},
				{ID_sc_data, fieldinfo_sc_data}
			}
		};
		ReflMngr::Instance().typeinfos.emplace(ID_A, std::move(typeinfo));
	}

	A a;
	ObjectPtr ptr{ ID_A, &a };

	for (const auto& [ID_field, fieldinfo] : ReflMngr::Instance().typeinfos.at(ID_A).fieldinfos) {
		auto field_name = ReflMngr::Instance().nregistry.Nameof(ID_field);
		std::cout << field_name << std::endl;
	}

	ReflMngr::Instance().RWVar(ptr, ID_data).As<int>() = 10;
	ReflMngr::Instance().RWVar(ptr, ID_s_data).As<int>() = 20;

	std::cout << ReflMngr::Instance().RVar(ptr, ID_data).As<int>() << std::endl;
	std::cout << ReflMngr::Instance().RVar(ptr, ID_c_data).As<int>() << std::endl;
	std::cout << ReflMngr::Instance().RVar(ptr, ID_s_data).As<int>() << std::endl;
	std::cout << ReflMngr::Instance().RVar(ptr, ID_sc_data).As<int>() << std::endl;

	return 0;
}
