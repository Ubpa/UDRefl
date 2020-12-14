#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Vec {
	float x;
	float y;
	float norm() const noexcept {
		return std::sqrt(x * x + y * y);
	}
};

int main() {
	ReflMngr::Instance().RegisterTypeAuto<Vec>();
	ReflMngr::Instance().AddField<&Vec::x>("x");
	ReflMngr::Instance().AddField<&Vec::y>("y");
	ReflMngr::Instance().AddMethod<&Vec::norm>("norm");

	// [ or ]
	// ObjectPtr v = ReflMngr::Instance().New(TypeID::of<Vec>);
	// // do something
	// ReflMngr::Instance().Delete(v);
	SharedObject v = ReflMngr::Instance().MakeShared(TypeID::of<Vec>);

	v->RWVar(StrID{ "x" }).As<float>() = 3.f;
	v->RWVar(StrID{ "y" }).As<float>() = 4.f;

	std::cout << "x: " << v->RVar("x").As<float>() << std::endl;
	std::cout << "norm: " << v->Invoke<float>("norm") << std::endl;
	
	for (auto field : ReflMngr::Instance().GetFields(TypeID::of<Vec>))
		std::cout << ReflMngr::Instance().nregistry.Nameof(field.ID) << std::endl;

	for (auto method : ReflMngr::Instance().GetMethods(TypeID::of<Vec>))
		std::cout << ReflMngr::Instance().nregistry.Nameof(method.ID) << std::endl;

	for (const auto& [type, field, var] : v->GetTypeFieldRVars()) {
		std::cout
			<< ReflMngr::Instance().nregistry.Nameof(field.ID)
			<< ": " << var.As<float>()
			<< std::endl;
		return true;
	}
}
