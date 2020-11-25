#include <UDRefl/UDRefl.h>

#include <iostream>

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
	size_t ID_Vec = NameRegistry::Instance().Register("Vec");
	size_t ID_const_Vec_ptr = NameRegistry::Instance().Register("const Vec*");
	size_t ID_p = NameRegistry::Instance().Register("p");
	size_t ID_float = NameRegistry::Instance().Register("float");
	size_t ID_x = NameRegistry::Instance().Register("x");
	size_t ID_y = NameRegistry::Instance().Register("y");
	size_t ID_Norm2 = NameRegistry::Instance().Register("Norm2");
	size_t ID_NormalizeSelf = NameRegistry::Instance().Register("NormalizeSelf");
	size_t ID_operator_add_assign = NameRegistry::Instance().Register("operator+=");

	{ // register Vec
		FieldPtr ptr_x{ ID_Vec, ID_float, offsetof(Vec, x) };
		FieldPtr ptr_y{ ID_Vec, ID_float, offsetof(Vec, y) };

		auto Norm2 = [](ConstObjectPtr obj, ArgsView) {
			assert(obj.GetID() == NameRegistry::Instance().GetID("Vec"));
			return obj.As<Vec>().Norm2();
		};
		ConstMethod method_Norm2{ {}, Norm2 };
		auto NormalizeSelf = [](ObjectPtr obj, ArgsView) {
			assert(obj.GetID() == NameRegistry::Instance().GetID("Vec"));
			obj.As<Vec>().NormalizeSelf();
			return nullptr;
		};
		Method method_NormalizeSelf{ {}, NormalizeSelf };
		auto operator_add_assign = [](ObjectPtr obj, ArgsView args) {
			assert(obj.GetID() == NameRegistry::Instance().GetID("Vec"));
			return &(obj.As<Vec>() += *args.ConstAt(0).As<const Vec*>());
		};
		Parameter param{
			ID_const_Vec_ptr,
			ID_p,
			true,
			sizeof(const Vec*),
			alignof(const Vec*)
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
		TypeInfoMngr::Instance().typeinfos.emplace(ID_Vec, std::move(typeinfo));
	}

	Vec v{ 2.f,3.f };
	ObjectPtr ptr{ ID_Vec , &v };

	TypeInfoMngr::Instance().typeinfos.at(ID_Vec).methodinfos.find(ID_NormalizeSelf)->second.method.Invoke(ptr);
	std::cout << v.x << ", " << v.y << std::endl;

	{
		auto rst = TypeInfoMngr::Instance().typeinfos.at(ID_Vec).cmethodinfos.find(ID_Norm2)->second.method.Invoke(ptr);
		std::cout << std::any_cast<float>(rst) << std::endl; 
	}

	{
		Vec w{ 10.f,10.f };
		std::uint8_t buffer[sizeof(Vec*)];
		reinterpret_cast<Vec*&>(buffer) = &w;
		auto rst = TypeInfoMngr::Instance().typeinfos.at(ID_Vec).methodinfos.find(ID_operator_add_assign)->second.method.Invoke(ptr, buffer);
		auto pw = std::any_cast<Vec*>(rst);
		std::cout << pw->x << ", " << pw->y << std::endl;
	}

	return 0;
}
