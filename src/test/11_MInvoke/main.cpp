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
	ReflMngr::Instance().RegisterTypeAuto<Vec>();
	ReflMngr::Instance().AddField<&Vec::x>("x");
	ReflMngr::Instance().AddField<&Vec::y>("y");
	ReflMngr::Instance().AddMethod<&Vec::norm>("norm");
	ReflMngr::Instance().AddMethod<&Vec::copy>("copy");
	ReflMngr::Instance().AddMethod<&Vec::operator+>(StrIDRegistry::Meta::operator_add);

	// [ or ]
	// ObjectPtr v = ReflMngr::Instance().New(TypeID::of<Vec>);
	// // do something
	// ReflMngr::Instance().Delete(v);
	SharedObject v = ReflMngr::Instance().MakeShared(TypeID::of<Vec>);

	v->RWVar(StrID{ "x" }).As<float>() = 3.f;
	v->RWVar(StrID{ "y" }).As<float>() = 4.f;

	std::cout << "x: " << v->RVar(StrID{ "x" }).As<float>() << std::endl;
	std::cout << "norm: " << v->Invoke<float>(StrID{ "norm" }) << std::endl;
	
	ReflMngr::Instance().ForEachField(
		TypeID::of<Vec>,
		[](TypeRef type, FieldRef field) {
			std::cout << ReflMngr::Instance().nregistry.Nameof(field.ID) << std::endl;
			return true;
		}
	);

	ReflMngr::Instance().ForEachMethod(
		TypeID::of<Vec>,
		[](TypeRef type, MethodRef method) {
			std::cout << ReflMngr::Instance().nregistry.Nameof(method.ID) << std::endl;
			return true;
		}
	);

	v->ForEachRVar(
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
			return true;
		}
	);

	auto w = v->DMInvoke<const Vec&>(StrIDRegistry::MetaID::operator_add, v.As<Vec>());

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
