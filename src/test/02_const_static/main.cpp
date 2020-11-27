#include <UDRefl/UDRefl.h>

#include <iostream>
#include <cassert>

using namespace Ubpa::UDRefl;

struct A {
	int data{ 1 };
	const int c_data{ 2 };
	inline static int s_data{ 3 };
	inline static const int sc_data{ 4 };
};

int main() {
	size_t ID_A = ReflMngr::Instance().registry.Register("A");
	size_t ID_int = ReflMngr::Instance().registry.Register("int");
	size_t ID_data = ReflMngr::Instance().registry.Register("data");
	size_t ID_c_data = ReflMngr::Instance().registry.Register("c_data");
	size_t ID_s_data = ReflMngr::Instance().registry.Register("s_data");
	size_t ID_sc_data = ReflMngr::Instance().registry.Register("sc_data");

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
			{}, // attrs
			{
				{ID_data, fieldinfo_data},
				{ID_c_data, fieldinfo_c_data},
				{ID_s_data, fieldinfo_s_data},
				{ID_sc_data, fieldinfo_sc_data}
			}, // fields
		};
		ReflMngr::Instance().typeinfos.emplace(ID_A, std::move(typeinfo));
	}

	A a;
	ObjectPtr ptr{ ID_A, &a };

	for (const auto& [ID_field, fieldinfo] : ReflMngr::Instance().typeinfos.at(ID_A).fieldinfos) {
		auto field_name = ReflMngr::Instance().registry.Nameof(ID_field);
		std::cout << field_name << std::endl;
	}

	ReflMngr::Instance().RWField(ptr, ID_data).As<int>() = 10;
	ReflMngr::Instance().RWField(ptr, ID_s_data).As<int>() = 20;

	std::cout << ReflMngr::Instance().RField(ptr, ID_data).As<int>() << std::endl;
	std::cout << ReflMngr::Instance().RField(ptr, ID_c_data).As<int>() << std::endl;
	std::cout << ReflMngr::Instance().RField(ptr, ID_s_data).As<int>() << std::endl;
	std::cout << ReflMngr::Instance().RField(ptr, ID_sc_data).As<int>() << std::endl;

	return 0;
}
