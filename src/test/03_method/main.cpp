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
	size_t ID_Vec = ReflMngr::Instance().registry.Register("Vec");
	size_t ID_const_Vec_ptr = ReflMngr::Instance().registry.Register("const Vec*");
	size_t ID_p = ReflMngr::Instance().registry.Register("p");
	size_t ID_float = ReflMngr::Instance().registry.Register("float");
	size_t ID_x = ReflMngr::Instance().registry.Register("x");
	size_t ID_y = ReflMngr::Instance().registry.Register("y");
	size_t ID_Norm2 = ReflMngr::Instance().registry.Register("Norm2");
	size_t ID_NormalizeSelf = ReflMngr::Instance().registry.Register("NormalizeSelf");
	size_t ID_operator_add_assign = ReflMngr::Instance().registry.Register("operator+=");

	{ // register Vec
		FieldPtr ptr_x{ ID_float, offsetof(Vec, x) };
		FieldPtr ptr_y{ ID_float, offsetof(Vec, y) };

		auto Norm2 = [](const void* obj, ArgsView) -> std::any {
			return reinterpret_cast<const Vec*>(obj)->Norm2();
		};
		ConstMethod method_Norm2{ {}, Norm2 };
		auto NormalizeSelf = [](void* obj, ArgsView) -> std::any {
			reinterpret_cast<Vec*>(obj)->NormalizeSelf();
			return {};
		};
		Method method_NormalizeSelf{ {}, NormalizeSelf };
		auto operator_add_assign = [](void* obj, ArgsView args) -> std::any {
			return &(*reinterpret_cast<Vec*>(obj) += *args.At(0).As<const Vec*>());
		};
		Parameter param{
			ID_const_Vec_ptr,
			sizeof(const Vec*),
			alignof(const Vec*),
			ID_p
		};
		ParamList paramList{ { param } };
		Method method_operator_add_assign{ paramList, operator_add_assign };
		
		FieldInfo fieldinfo_x{ ptr_x };
		FieldInfo fieldinfo_y{ ptr_y };
		MethodInfo methodinfo_NormalizeSelf{ method_NormalizeSelf };
		MethodInfo methodinfo_operator_add_assign{ method_operator_add_assign };
		ConstMethodInfo cmethodinfo_Norm2{ method_Norm2 };

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
				{ID_NormalizeSelf, methodinfo_NormalizeSelf},
				{ID_operator_add_assign, methodinfo_operator_add_assign}
			},
			{ // cmethods
				{ID_Norm2, cmethodinfo_Norm2}
			},
		};
		ReflMngr::Instance().typeinfos.emplace(ID_Vec, std::move(typeinfo));
	}

	Vec v{ 2.f,3.f };
	ObjectPtr ptr{ ID_Vec , &v };

	ReflMngr::Instance().Invoke(ptr, ID_NormalizeSelf, {}, nullptr);
	std::cout << v.x << ", " << v.y << std::endl;

	{
		auto [success, rst] = ReflMngr::Instance().Invoke(ptr, ID_Norm2, {}, nullptr);
		assert(success);
		std::cout << std::any_cast<float>(rst) << std::endl; 
	}

	{
		Vec w{ 10.f,10.f };
		std::uint8_t buffer[sizeof(const Vec*)];
		*reinterpret_cast<const Vec**>(buffer) = &w;
		auto [success, rst] = ReflMngr::Instance().Invoke(ptr, ID_operator_add_assign, std::array{ ID_const_Vec_ptr }, buffer);
		assert(success);
		auto pw = std::any_cast<Vec*>(rst);
		std::cout << pw->x << ", " << pw->y << std::endl;
	}
	
	return 0;
}
