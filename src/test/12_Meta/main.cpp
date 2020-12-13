#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Vec {
	float x;
	float y;
	Vec operator+(SharedConstObject v) const noexcept {
		Vec rst;
		rst.x = x + v.As<Vec>().x;
		rst.y = y + v.As<Vec>().y;
		return rst;
	}
	Vec operator+(float k) const noexcept {
		Vec rst;
		rst.x = x + k;
		rst.y = y + k;
		return rst;
	}

	Vec operator+(ConstObjectPtr v) const noexcept {
		Vec rst;
		rst.x = x + v.As<Vec>().x;
		rst.y = y + v.As<Vec>().y;
		return rst;
	}
	Vec operator-(ConstObjectPtr v) const noexcept {
		Vec rst;
		rst.x = x - v.As<Vec>().x;
		rst.y = y - v.As<Vec>().x;
		return rst;
	}
	Vec operator*(ConstObjectPtr v) const noexcept {
		Vec rst;
		rst.x = x * v.As<Vec>().x;
		rst.y = y * v.As<Vec>().x;
		return rst;
	}
	Vec operator/(ConstObjectPtr v) const noexcept {
		Vec rst;
		rst.x = x / v.As<Vec>().x;
		rst.y = y / v.As<Vec>().x;
		return rst;
	}
	float operator[](int v) const noexcept {
		if (v == 0)
			return x;
		else if (v == 1)
			return y;
		else
			return 0.f;
	}

	void operator()(ConstObjectPtr v) const noexcept {
		std::cout << "call: " << v.As<Vec>().x << std::endl;
	}
};

int main() {
	ReflMngr::Instance().RegisterTypeAuto<Vec>();
	ReflMngr::Instance().AddConstructor<Vec, float, float>();
	ReflMngr::Instance().AddField<&Vec::x>("x");
	ReflMngr::Instance().AddField<&Vec::y>("y");
	ReflMngr::Instance().AddMethod<MemFuncOf<Vec(ConstObjectPtr)const noexcept>::run(&Vec::operator+)>(StrIDRegistry::Meta::operator_add);
	ReflMngr::Instance().AddMethod<MemFuncOf<Vec(SharedConstObject)const noexcept>::run(&Vec::operator+)>(StrIDRegistry::Meta::operator_add);
	ReflMngr::Instance().AddMethod<MemFuncOf<Vec(float)const noexcept>::run(&Vec::operator+)>(StrIDRegistry::Meta::operator_add);
	ReflMngr::Instance().AddMethod<&Vec::operator- >(StrIDRegistry::Meta::operator_minus);
	ReflMngr::Instance().AddMethod<&Vec::operator* >(StrIDRegistry::Meta::operator_mul);
	ReflMngr::Instance().AddMethod<&Vec::operator/ >(StrIDRegistry::Meta::operator_div);
	ReflMngr::Instance().AddMethod<&Vec::operator[] >(StrIDRegistry::Meta::operator_subscript);
	ReflMngr::Instance().AddMethod<&Vec::operator() >(StrIDRegistry::Meta::operator_call);

	SharedObject v = ReflMngr::Instance().MakeShared(TypeID::of<Vec>, 3.f, 4.f);

	ObjectPtr pv = v;
	SharedObject w0 = v + v;
	SharedObject w1 = v + pv;
	float fv = 1.f;
	SharedObject w2 = v + 1.f;
	SharedObject w3 = v - pv;
	SharedObject w4 = v * pv;
	SharedObject w5 = v / pv;

	SharedObject w6 = v[1];
	v(pv);

	for (const auto& w : std::array{ w0,w1,w2,w3,w4,w5,w6 }) {
		w->ForEachRVar(
			[](TypeRef type, FieldRef field, ConstObjectPtr var) {
				std::cout
					<< ReflMngr::Instance().nregistry.Nameof(field.ID)
					<< ": " << var.As<float>()
					<< std::endl;
				return true;
			}
		);
	}
}
