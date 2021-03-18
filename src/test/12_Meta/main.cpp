#include <UDRefl/UDRefl.hpp>

#include <array>
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
	Vec& operator+=(const Vec& v) noexcept {
		x += v.x;
		y += v.y;
		return *this;
	}
	float operator[](std::size_t v) const noexcept {
		if (v == 0)
			return x;
		else if (v == 1)
			return y;
		else
			return 0.f;
	}
};

int main() {
	Mngr.RegisterType<Vec>();
	Mngr.AddConstructor<Vec, float, float>();
	Mngr.AddField<&Vec::x>("x");
	Mngr.AddField<&Vec::y>("y");
	Mngr.AddMethod<MemFuncOf<Vec, Vec(float)const noexcept>::get(&Vec::operator+)>(NameIDRegistry::Meta::operator_add);

	SharedObject v = Mngr.MakeShared(Type_of<Vec>, TempArgsView{ 3.f, 4.f });

	ObjectView pv = v;
	SharedObject w0 = v + v;
	SharedObject w1 = v + pv;
	SharedObject w2 = v + 1.f;
	SharedObject w3 = v - pv;
	SharedObject w4 = v * pv;
	SharedObject w5 = v / pv;
	SharedObject w6 = v + Vec{ 1.f,2.f };

	for (const auto& w : std::array{ w0,w1,w2,w3,w4,w5,w6 }) {
		for (const auto& [name, var] : w.GetVars())
			std::cout << name << ": " << var << std::endl;
	}

	SharedObject ele_1 = (v += Vec{ 10.f,10.f })[static_cast<std::size_t>(1)];
	std::cout << "ele_1: " << ele_1 << std::endl;
}
