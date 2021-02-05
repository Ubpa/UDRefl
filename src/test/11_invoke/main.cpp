#include <UDRefl/UDRefl.h>

#include <iostream>
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

	SharedObject v = Mngr.MakeShared(TypeID_of<Vec>);

	v.Var("x") = 3.f;
	v.Var("y") = 4.f;

	for (auto method : Mngr.GetMethods(TypeID_of<Vec>))
		std::cout << Mngr.nregistry.Nameof(method.ID) << std::endl;

	for (const auto& [type, field, var] : v.GetTypeFieldVars()) {
		std::cout
			<< Mngr.nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}

	auto w0 = v.MInvoke(StrIDRegistry::MetaID::operator_add, std::pmr::get_default_resource(), v.As<Vec>());
	auto w1 = v.DMInvoke(StrIDRegistry::MetaID::operator_add, v.As<Vec>());
	auto w2 = v.ADMInvoke(StrIDRegistry::MetaID::operator_add, v);

	std::array arr_w = { w0,w1,w2 };
	for (auto w : arr_w) {
		for (const auto& [type, field, var] : w.GetTypeFieldVars()) {
			std::cout
				<< Mngr.nregistry.Nameof(field.ID)
				<< ": " << var
				<< std::endl;
		}
	}
}
