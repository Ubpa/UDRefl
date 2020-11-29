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
	auto ID_Vec = ReflMngr::Instance().tregistry.Register("Vec");
	auto ID_const_Vec_ptr = ReflMngr::Instance().tregistry.Register("const Vec*");
	auto ID_Vec_ptr = ReflMngr::Instance().tregistry.Register("Vec*");
	auto ID_float = ReflMngr::Instance().tregistry.Register("float");

	auto ID_p = ReflMngr::Instance().nregistry.Register("p");
	auto ID_d = ReflMngr::Instance().nregistry.Register("d");
	auto ID_x = ReflMngr::Instance().nregistry.Register("x");
	auto ID_y = ReflMngr::Instance().nregistry.Register("y");
	auto ID_operator_add_assign = ReflMngr::Instance().nregistry.Register("operator+=");

	{ // register Vec
		FieldPtr ptr_x{ ID_float, offsetof(Vec, x) };
		FieldPtr ptr_y{ ID_float, offsetof(Vec, y) };

		auto operator_add_assign_1 = [](void* obj, ArgsView args, void* result_buffer) {
			auto v = reinterpret_cast<Vec*>(obj);
			*reinterpret_cast<Vec**>(result_buffer) =
				&(*v += *args.At(0).As<const Vec*>());
			return destructor<Vec*>();
		};
		Method method_operator_add_assign_1{ operator_add_assign_1, {{
			{
				ID_const_Vec_ptr,
				sizeof(const Vec*),
				alignof(const Vec*),
				ID_p
			}
		}}, {ID_Vec_ptr, sizeof(Vec*), alignof(Vec*)} };

		auto operator_add_assign_2 = [](void* obj, ArgsView args, void* result_buffer) {
			assert(args.GetParamList().GetParameters().at(0).typeID == ReflMngr::Instance().tregistry.GetID("float"));
			*reinterpret_cast<Vec**>(result_buffer) =
				&(*reinterpret_cast<Vec*>(obj) += args.At(0).As<float>());
			return destructor<Vec*>();
		};
		Method method_operator_add_assign_2{ operator_add_assign_2, {{
			{
				ID_float,
				sizeof(float),
				alignof(float),
				ID_d
			}
		}}, {ID_Vec_ptr, sizeof(Vec*), alignof(Vec*)} };

		FieldInfo fieldinfo_x{ ptr_x };
		FieldInfo fieldinfo_y{ ptr_y };
		MethodInfo methodinfo_operator_add_assign_1{ method_operator_add_assign_1 };
		MethodInfo methodinfo_operator_add_assign_2{ method_operator_add_assign_2 };

		TypeInfo typeinfo{
			sizeof(Vec),
			alignof(Vec),
			{ // fields
				{ID_x, fieldinfo_x},
				{ID_y, fieldinfo_y}
			},
			{ // methods
				{ID_operator_add_assign, methodinfo_operator_add_assign_1},
				{ID_operator_add_assign, methodinfo_operator_add_assign_2}
			}
		};
		ReflMngr::Instance().typeinfos.emplace(ID_Vec, std::move(typeinfo));
	}

	Vec v{ 2.f,3.f };
	ObjectPtr ptr{ ID_Vec , &v };

	{
		Vec w{ 10.f,10.f };
		std::uint8_t args_buffer[sizeof(const Vec*)];
		std::uint8_t result_buffer[sizeof(Vec*)];
		*reinterpret_cast<const Vec**>(args_buffer) = &w;
		auto [success, typeID, destructor]
			= ReflMngr::Instance().Invoke(ptr, ID_operator_add_assign, std::array{ ID_const_Vec_ptr }, args_buffer, result_buffer);
		assert(success);
		assert(typeID == ID_Vec_ptr);
		assert(!destructor);
		auto pw = *reinterpret_cast<Vec**>(result_buffer);
		std::cout << pw->x << ", " << pw->y << std::endl;
	}

	{
		std::uint8_t args_buffer[sizeof(float)];
		std::uint8_t result_buffer[sizeof(Vec*)];
		*reinterpret_cast<float*>(args_buffer) = 2.f;
		auto [success, typeID, destructor]
			= ReflMngr::Instance().Invoke(ptr, ID_operator_add_assign, std::array{ ID_float }, args_buffer, result_buffer);
		assert(success);
		assert(typeID == ID_Vec_ptr);
		assert(!destructor);
		auto pw = *reinterpret_cast<Vec**>(result_buffer);
		std::cout << pw->x << ", " << pw->y << std::endl;
	}
	
	return 0;
}
