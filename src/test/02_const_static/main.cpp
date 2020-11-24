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
	size_t ID_A = NameRegistry::Instance().Register("A");
	size_t ID_int = NameRegistry::Instance().Register("int");
	size_t ID_data = NameRegistry::Instance().Register("data");
	size_t ID_c_data = NameRegistry::Instance().Register("c_data");
	size_t ID_s_data = NameRegistry::Instance().Register("s_data");
	size_t ID_sc_data = NameRegistry::Instance().Register("sc_data");

	{ // register Point
		FieldPtr ptr_data{ ID_A, ID_int, offsetof(A, data) };
		ConstFieldPtr ptr_c_data{ ID_A, ID_int, offsetof(A, c_data) };
		ObjectPtr ptr_s_data{ ID_int, &A::s_data };
		ConstObjectPtr ptr_sc_data{ ID_int, &A::sc_data };

		FieldInfo fieldinfo_data{ ptr_data };
		ConstFieldInfo fieldinfo_c_data{ ptr_c_data };
		StaticFieldInfo fieldinfo_s_data{ ptr_s_data };
		StaticConstFieldInfo fieldinfo_sc_data{ ptr_sc_data };
		TypeInfo typeinfo{
			{}, // attrs
			{{ID_data, fieldinfo_data}}, // fields
			{{ID_c_data, fieldinfo_c_data}}, // const fields
			{{ID_s_data, fieldinfo_s_data}}, // static fields
			{{ID_sc_data, fieldinfo_sc_data}}, // static const fields
		};
		TypeInfoMngr::Instance().typeinfos.emplace(ID_A, std::move(typeinfo));
	}

	A a;
	ObjectPtr ptr{ ID_A, &a };

	for (const auto& [ID_field, fieldinfo] : TypeInfoMngr::Instance().typeinfos.at(ID_A).fieldinfos) {
		auto field = fieldinfo.fieldptr.Map(ptr);
		auto field_name = NameRegistry::Instance().Nameof(ID_field);
		if (field.GetID() == ID_int)
			std::cout << field_name << ": " << field.As<int>() << std::endl;
	}

	for (const auto& [ID_field, cfieldinfo] : TypeInfoMngr::Instance().typeinfos.at(ID_A).cfieldinfos) {
		auto field = cfieldinfo.fieldptr.Map(ptr);
		auto field_name = NameRegistry::Instance().Nameof(ID_field);
		if (field.GetID() == ID_int)
			std::cout << field_name << ": " << field.As<int>() << std::endl;
	}

	for (const auto& [ID_field, sfieldinfo] : TypeInfoMngr::Instance().typeinfos.at(ID_A).sfieldinfos) {
		auto field_name = NameRegistry::Instance().Nameof(ID_field);
		if (sfieldinfo.objptr.GetID() == ID_int)
			std::cout << field_name << ": " << sfieldinfo.objptr.As<int>() << std::endl;
	}

	for (const auto& [ID_field, scfieldinfo] : TypeInfoMngr::Instance().typeinfos.at(ID_A).scfieldinfos) {
		auto field_name = NameRegistry::Instance().Nameof(ID_field);
		if (scfieldinfo.objptr.GetID() == ID_int)
			std::cout << field_name << ": " << scfieldinfo.objptr.As<int>() << std::endl;
	}

	return 0;
}
