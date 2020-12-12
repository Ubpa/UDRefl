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

	ReflMngr::Instance().RWVar(v, StrID{"x"}).As<float>() = 3.f;
	ReflMngr::Instance().RWVar(v, StrID{"y"}).As<float>() = 4.f;

	std::cout << "x: " << ReflMngr::Instance().RVar(v, StrID{"x"}).As<float>() << std::endl;
	std::cout << "norm: " << ReflMngr::Instance().Invoke<float>(v.AsObjectPtr(), StrID{"norm"}) << std::endl;
	
	ReflMngr::Instance().ForEachField(
		TypeID::of<Vec>,
		[](TypeRef type, FieldRef field) {
			std::cout << ReflMngr::Instance().nregistry.Nameof(field.ID) << std::endl;
			return true;
		}
	);

	ReflMngr::Instance().ForEachMethod(
		TypeID::of<Vec>,
		[](TypeRef type, Method method) {
			std::cout << ReflMngr::Instance().nregistry.Nameof(method.ID) << std::endl;
			return true;
		}
	);

	ReflMngr::Instance().ForEachRVar(
		v, // ObjectPtr
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
			return true;
		}
	);
}
