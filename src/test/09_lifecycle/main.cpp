#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa::UDRefl;

struct Point {
	Point() {
		std::cout << "Point ctor" << std::endl;
	}
	~Point() {
		std::cout << "Point dtor" << std::endl;
	}
	float x, y;
};

int main() {
	auto ID_Point = ReflMngr::Instance().tregistry.Register<Point>();
	auto ID_float = ReflMngr::Instance().tregistry.Register<float>();
	auto ID_x = ReflMngr::Instance().nregistry.Register("x");
	auto ID_y = ReflMngr::Instance().nregistry.Register("y");
	auto ID_ctor = ReflMngr::Instance().nregistry.Register(StrIDRegistry::Meta::ctor);
	auto ID_dtor = ReflMngr::Instance().nregistry.Register(StrIDRegistry::Meta::dtor);

	ReflMngr::Instance().typeinfos[ID_Point] = {
		sizeof(Point),
		alignof(Point),
		{ // fieldinfos
			{ ID_x, { { ID_float, offsetof(Point, x) } }},
			{ ID_y, { { ID_float, offsetof(Point, y) } }}
		},
		{ // methods
			ReflMngr::Instance().GenerateConstructor<Point>(),
			ReflMngr::Instance().GenerateDestructor<Point>()
		}
	};
	
	SharedObject p = ReflMngr::Instance().MakeShared(ID_Point);
	ReflMngr::Instance().RWVar(p, ID_x).As<float>() = 1.f;
	ReflMngr::Instance().RWVar(p, ID_y).As<float>() = 2.f;

	ReflMngr::Instance().ForEachRVar( p,
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
			return true;
		}
	);
}
