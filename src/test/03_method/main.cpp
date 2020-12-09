#include <UDRefl/UDRefl.h>

#include <iostream>
#include <array>

using namespace Ubpa::UDRefl;

struct Vec {
	float x;
	float y;

	float Norm2() const noexcept {
		return x * x + y * y;
	}

	void NormalizeSelf() noexcept {
		float n = std::sqrt(Norm2());
		assert(n != 0);
		x /= n;
		y /= n;
	}

	Vec& operator+=(const Vec& p) noexcept {
		x += p.x;
		y += p.y;
		return *this;
	}
};

int main() {
	auto ID_Vec = ReflMngr::Instance().tregistry.GetID<Vec>();
	auto ID_const_Vec_lref = ReflMngr::Instance().tregistry.GetID<const Vec&>();
	auto ID_Vec_lref = ReflMngr::Instance().tregistry.GetID<Vec&>();
	auto ID_float = ReflMngr::Instance().tregistry.GetID<float>();

	auto ID_p = ReflMngr::Instance().nregistry.GetID("p");
	auto ID_x = ReflMngr::Instance().nregistry.GetID("x");
	auto ID_y = ReflMngr::Instance().nregistry.GetID("y");
	auto ID_Norm2 = ReflMngr::Instance().nregistry.GetID("Norm2");
	auto ID_NormalizeSelf = ReflMngr::Instance().nregistry.GetID("NormalizeSelf");
	auto ID_operator_assign_add = ReflMngr::Instance().nregistry.GetID(NameRegistry::Meta::operator_assign_add);

	{ // register Vec
		FieldPtr ptr_x{ ID_float, offsetof(Vec, x) };
		FieldPtr ptr_y{ ID_float, offsetof(Vec, y) };
		auto Norm2 = [](const void* obj, ArgsView, void* result_buffer) {
			return wrap_function<&Vec::Norm2>()(obj, nullptr, result_buffer);
		};
		MethodPtr method_Norm2{
			Norm2,
			{ID_float, sizeof(float), alignof(float)}
		};

		auto NormalizeSelf = [](void* obj, ArgsView, void*) {
			return wrap_function<&Vec::NormalizeSelf>()(obj, nullptr, nullptr);
		};
		MethodPtr method_NormalizeSelf{ NormalizeSelf };

		auto operator_add_assign = [](void* obj, ArgsView args, void* result_buffer) {
			return wrap_function<&Vec::operator+=>()(obj, args.GetBuffer(), result_buffer);
		};
		MethodPtr method_operator_add_assign{
			operator_add_assign,
			{ID_Vec_lref, sizeof(Vec*), alignof(Vec*)},
			{{
				{
					ID_const_Vec_lref,
					sizeof(const Vec*),
					alignof(const Vec*),
					ID_p
				}
			}}
		};
		
		FieldInfo fieldinfo_x{ ptr_x };
		FieldInfo fieldinfo_y{ ptr_y };
		MethodInfo methodinfo_NormalizeSelf{ method_NormalizeSelf };
		MethodInfo methodinfo_operator_add_assign{ method_operator_add_assign };
		MethodInfo methodinfo_Norm2{ method_Norm2 };

		TypeInfo typeinfo{
			sizeof(Vec),
			alignof(Vec),
			{ // fieldinfos
				{ID_x, fieldinfo_x},
				{ID_y, fieldinfo_y}
			},
			{ // methodinfos
				{ID_NormalizeSelf, methodinfo_NormalizeSelf},
				{ID_operator_assign_add, methodinfo_operator_add_assign},
				{ID_Norm2, methodinfo_Norm2}
			},
		};
		ReflMngr::Instance().typeinfos.emplace(ID_Vec, std::move(typeinfo));
	}

	Vec v{ 2.f,3.f };
	ObjectPtr ptr{ ID_Vec , &v };

	ReflMngr::Instance().Invoke(ptr, ID_NormalizeSelf, {}, nullptr, nullptr);
	std::cout << v.x << ", " << v.y << std::endl;

	{
		std::uint8_t result_buffer[sizeof(float)];
		auto [success, typeID, destructor] = ReflMngr::Instance().Invoke(ptr, ID_Norm2, {}, nullptr, result_buffer);
		assert(success);
		assert(typeID == ID_float);
		assert(!destructor);
		std::cout << *reinterpret_cast<float*>(result_buffer) << std::endl;
	}

	{
		Vec w{ 10.f,10.f };
		std::array argTypeIDs{ ID_const_Vec_lref };
		std::uint8_t args_buffer[sizeof(const Vec*)];
		buffer_get<const Vec*>(args_buffer, 0) = &w;
		std::uint8_t result_buffer[sizeof(Vec*)];
		auto [success, typeID, destructor]
			= ReflMngr::Instance().Invoke(ptr, ID_operator_assign_add, argTypeIDs, args_buffer, result_buffer);
		assert(success);
		assert(typeID == ID_Vec_lref);
		assert(!destructor);
		auto pw = buffer_as<Vec*>(result_buffer);
		std::cout << pw->x << ", " << pw->y << std::endl;
	}

	{
		Vec w{ 10.f,10.f };
		std::array argTypeIDs{ ID_const_Vec_lref };
		std::uint8_t args_buffer[sizeof(const Vec*)];
		buffer_get<const Vec*>(args_buffer, 0) = &w;
		auto& v = ReflMngr::Instance().InvokeRet<Vec&>(ptr, ID_operator_assign_add, argTypeIDs, args_buffer);
		std::cout << v.x << ", " << v.y << std::endl;
	}

	{
		Vec w{ 10.f,10.f };
		auto& v = ReflMngr::Instance().Invoke<Vec&, const Vec&>(ptr, ID_operator_assign_add, w);
		std::cout << v.x << ", " << v.y << std::endl;
	}
	
	return 0;
}
