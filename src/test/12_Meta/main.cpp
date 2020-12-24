#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Vec {
	float x;
	float y;
	Vec operator+(float k) const noexcept {
		Vec rst;
		rst.x = x + k;
		rst.y = y + k;
		return rst;
	}
	Vec operator+(const Vec& v) const noexcept {
		Vec rst;
		rst.x = x + v.x;
		rst.y = y + v.y;
		return rst;
	}
	Vec operator-(const Vec& v) const noexcept {
		Vec rst;
		rst.x = x - v.x;
		rst.y = y - v.y;
		return rst;
	}
	Vec operator*(const Vec& v) const noexcept {
		Vec rst;
		rst.x = x * v.x;
		rst.y = y * v.y;
		return rst;
	}
	Vec operator/(const Vec& v) const noexcept {
		Vec rst;
		rst.x = x / v.x;
		rst.y = y / v.y;
		return rst;
	}
	float operator[](size_t v) const noexcept {
		if (v == 0)
			return x;
		else if (v == 1)
			return y;
		else
			return 0.f;
	}
};

int main() {
	ReflMngr::Instance().RegisterTypeAuto<Vec>();
	ReflMngr::Instance().AddConstructor<Vec, float, float>();
	ReflMngr::Instance().AddField<&Vec::x>("x");
	ReflMngr::Instance().AddField<&Vec::y>("y");
	ReflMngr::Instance().AddMethod<MemFuncOf<Vec(float)const noexcept>::get(&Vec::operator+)>(StrIDRegistry::Meta::operator_add);

	SharedObject v = ReflMngr::Instance().MakeShared(TypeID_of<Vec>, 3.f, 4.f);

	ObjectPtr pv = v;
	SharedObject w0 = v + v;
	SharedObject w1 = v + pv;
	float fv = 1.f;
	SharedObject w2 = v + 1.f;
	SharedObject w3 = v - pv;
	SharedObject w4 = v * pv;
	SharedObject w5 = v / pv;
	SharedObject w6 = v + Vec{ 1.f,2.f };

	v->Invoke("hello");
	v->Invoke(std::string_view{ "hello" });
	ReflMngr::Instance().AlignedFree(ReflMngr::Instance().AlignedMalloc(1, 1));
	SharedObject w7 = v[static_cast<size_t>(1)];
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
