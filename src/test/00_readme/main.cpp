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
	TypeID ID_Vec = ReflMngr::Instance().tregistry.Register<Vec>();

	ReflMngr::Instance().typeinfos[ID_Vec] = { // TypeInfo
		sizeof(Vec),  // size
		alignof(Vec), // alignment
		{ // field infos
			ReflMngr::Instance().GenerateField<&Vec::x>("x"), // x
			ReflMngr::Instance().GenerateField<&Vec::y>("y")  // y
		},
		{ // method infos
			ReflMngr::Instance().GenerateConstructor<Vec>(),        // default ctor
			ReflMngr::Instance().GenerateDestructor<Vec>(),         // dtor
			ReflMngr::Instance().GenerateMethod<&Vec::norm>("norm") // norm
		}
	};

	// [ or ]
	// ObjectPtr v = ReflMngr::Instance().New(ID_Vec);
	// // do something
	// ReflMngr::Instance().Delete(v);
	SharedObject v = ReflMngr::Instance().MakeShared(ID_Vec);

	ReflMngr::Instance().RWVar(v, StrID("x")).As<float>() = 3.f;
	ReflMngr::Instance().RWVar(v, StrID("y")).As<float>() = 4.f;

	std::cout << "x: " << ReflMngr::Instance().RVar(v, StrID("x")).As<float>() << std::endl;
	std::cout << "norm: " << ReflMngr::Instance().Invoke<float>(v.AsObjectPtr(), StrID{"norm"}) << std::endl;
	
	ReflMngr::Instance().ForEachField(
		ID_Vec, // TypeID
		[](Type type, Field field) {
			std::cout << ReflMngr::Instance().nregistry.Nameof(field.ID) << std::endl;
		}
	);

	ReflMngr::Instance().ForEachMethod(
		ID_Vec, // TypeID
		[](Type type, Method method) {
			std::cout << ReflMngr::Instance().nregistry.Nameof(method.ID) << std::endl;
		}
	);

	ReflMngr::Instance().ForEachRVar(
		v, // ObjectPtr
		[](Type type, Field field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
		}
	);
}
