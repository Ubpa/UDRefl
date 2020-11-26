#include <UDRefl/UDRefl.h>

#include <iostream>
#include <array>

using namespace Ubpa::UDRefl;

struct Vec {
	float x;
	float y;

	Vec& operator+=(const Vec& p) noexcept {
		std::cout << "Vec& operator+=(const Vec&) noexcept" << std::endl;
		x += p.x;
		y += p.y;
		return *this;
	}

	Vec& operator+=(float d) noexcept {
		std::cout << "Vec& operator+=(float) noexcept" << std::endl;
		x += d;
		y += d;
		return *this;
	}
};

int main() {
	size_t ID_Vec = ReflMngr::Instance().registry.Register("Vec");
	size_t ID_const_Vec_ptr = ReflMngr::Instance().registry.Register("const Vec*");
	size_t ID_p = ReflMngr::Instance().registry.Register("p");
	size_t ID_d = ReflMngr::Instance().registry.Register("d");
	size_t ID_float = ReflMngr::Instance().registry.Register("float");
	size_t ID_x = ReflMngr::Instance().registry.Register("x");
	size_t ID_y = ReflMngr::Instance().registry.Register("y");
	size_t ID_operator_add_assign = ReflMngr::Instance().registry.Register("operator+=");

	{ // register Vec
		FieldPtr ptr_x{ ID_float, offsetof(Vec, x) };
		FieldPtr ptr_y{ ID_float, offsetof(Vec, y) };

		auto operator_add_assign_1 = [](void* obj, ArgsView args) -> std::any {
			assert(args.GetParamList().GetParameters().at(0).typeID == ReflMngr::Instance().registry.GetID("const Vec*"));
			return &(*reinterpret_cast<Vec*>(obj) += *args.At(0).As<const Vec*>());
		};
		Parameter param_1{
			ID_const_Vec_ptr,
			sizeof(const Vec*),
			alignof(const Vec*),
			ID_p
		};
		ParamList paramList_1{ { param_1 } };
		Method method_operator_add_assign_1{ paramList_1, operator_add_assign_1 };

		auto operator_add_assign_2 = [](void* obj, ArgsView args) -> std::any {
			assert(args.GetParamList().GetParameters().at(0).typeID == ReflMngr::Instance().registry.GetID("float"));
			return &(*reinterpret_cast<Vec*>(obj) += args.At(0).As<float>());
		};
		Parameter param_2{
			ID_float,
			sizeof(float),
			alignof(float),
			ID_d
		};
		ParamList paramList_2{ { param_2 } };
		Method method_operator_add_assign_2{ paramList_2, operator_add_assign_2 };

		FieldInfo fieldinfo_x{ ptr_x };
		FieldInfo fieldinfo_y{ ptr_y };
		MethodInfo methodinfo_operator_add_assign_1{ method_operator_add_assign_1 };
		MethodInfo methodinfo_operator_add_assign_2{ method_operator_add_assign_2 };

		TypeInfo typeinfo{
			{}, // attrs
			{ // fields
				{ID_x, fieldinfo_x},
				{ID_y, fieldinfo_y}
			},
			{}, // cfields
			{}, // sfields
			{}, // scfields
			{ // methods
				{ID_operator_add_assign, methodinfo_operator_add_assign_1},
				{ID_operator_add_assign, methodinfo_operator_add_assign_2}
			},
			{} // cmethods
		};
		ReflMngr::Instance().typeinfos.emplace(ID_Vec, std::move(typeinfo));
	}

	Vec v{ 2.f,3.f };
	ObjectPtr ptr{ ID_Vec , &v };

	{
		Vec w{ 10.f,10.f };
		std::uint8_t buffer[sizeof(const Vec*)];
		*reinterpret_cast<const Vec**>(buffer) = &w;
		auto [success, rst] = ReflMngr::Instance().Invoke(ptr, ID_operator_add_assign, std::array{ ID_const_Vec_ptr }, buffer);
		assert(success);
		auto pw = std::any_cast<Vec*>(rst);
		std::cout << pw->x << ", " << pw->y << std::endl;
	}

	{
		std::uint8_t buffer[sizeof(float)];
		*reinterpret_cast<float*>(buffer) = 2.f;
		auto [success, rst] = ReflMngr::Instance().Invoke(ptr, ID_operator_add_assign, std::array{ ID_float }, buffer);
		assert(success);
		auto pw = std::any_cast<Vec*>(rst);
		std::cout << pw->x << ", " << pw->y << std::endl;
	}
	
	return 0;
}
