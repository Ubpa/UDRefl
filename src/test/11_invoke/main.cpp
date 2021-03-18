#include <UDRefl/UDRefl.hpp>

#include <iostream>
#include <array>
#include <cmath>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Vec {
	float x;
	float y;
	float norm() const noexcept {
		return std::sqrt(x * x + y * y);
	}
	Vec copy() const noexcept {
		return *this;
	}
	Vec operator+(const Vec& v) const noexcept {
		Vec rst;
		rst.x = x + v.x;
		rst.y = y + v.y;
		return rst;
	}
};

int main() {
	Mngr.RegisterType<Vec>();
	Mngr.AddField<&Vec::x>("x");
	Mngr.AddField<&Vec::y>("y");
	Mngr.AddMethod<&Vec::norm>("norm");
	Mngr.AddMethod<&Vec::copy>("copy");

	SharedObject v = Mngr.MakeShared(Type_of<Vec>);

	v.Var("x") = 3.f;
	v.Var("y") = 4.f;

	auto w0 = v.MInvoke(NameIDRegistry::Meta::operator_add, std::pmr::get_default_resource(), TempArgsView{ v });
	auto w1 = v.Invoke(NameIDRegistry::Meta::operator_add, TempArgsView{ v });

	std::array arr_w = { w0,w1 };
	for (auto w : arr_w) {
		for (const auto& [name, var] : w.GetVars())
			std::cout << name.GetView() << ": " << var << std::endl;
	}
}
