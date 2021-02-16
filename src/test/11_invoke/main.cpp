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
	Mngr->RegisterType<Vec>();
	Mngr->AddField<&Vec::x>("x");
	Mngr->AddField<&Vec::y>("y");
	Mngr->AddMethod<&Vec::norm>("norm");
	Mngr->AddMethod<&Vec::copy>("copy");

	SharedObject v = Mngr->MakeShared(Type_of<Vec>);

	v.Var("x") = 3.f;
	v.Var("y") = 4.f;

	for (const auto& method : Mngr->GetMethods(Type_of<Vec>))
		std::cout << method.name.GetView() << std::endl;

	for (const auto& [type, field, var] : v.GetTypeFieldVars()) {
		std::cout
			<< field.name.GetView()
			<< ": " << var
			<< std::endl;
	}

	auto w0 = v.MInvoke(NameIDRegistry::Meta::operator_add, std::pmr::get_default_resource(), std::pmr::get_default_resource(), MethodFlag::All, v.As<Vec>());
	auto w1 = v.DMInvoke(NameIDRegistry::Meta::operator_add, v.As<Vec>());
	auto w2 = v.ADMInvoke(NameIDRegistry::Meta::operator_add, v);

	std::array arr_w = { w0,w1,w2 };
	for (auto w : arr_w) {
		for (const auto& [type, field, var] : w.GetTypeFieldVars()) {
			std::cout
				<< field.name.GetView()
				<< ": " << var
				<< std::endl;
		}
	}
}
