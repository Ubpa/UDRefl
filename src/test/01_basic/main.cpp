#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa::UDRefl;

struct Point { float x, y; };

int main() {
	auto ID_Point = ReflMngr::Instance().tregistry.Register<Point>();
	auto ID_float = ReflMngr::Instance().tregistry.Register<float>();

	auto ID_x = ReflMngr::Instance().nregistry.Register("x");
	auto ID_y = ReflMngr::Instance().nregistry.Register("y");

	ReflMngr::Instance().typeinfos[ID_Point] = {
		sizeof(Point),
		alignof(Point),
		{ // fieldinfos
			{ ID_x, { { ID_float, offsetof(Point, x) } }},
			{ ID_y, { { ID_float, offsetof(Point, y) } }}
		}
	};
	
	Point p;
	ObjectPtr ptr{ ID_Point, &p };
	ReflMngr::Instance().RWVar(ptr, ID_x).As<float>() = 1.f;
	ReflMngr::Instance().RWVar(ptr, ID_y).As<float>() = 2.f;

	ReflMngr::Instance().ForEachRVar(
		ptr,
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
			return true;
		}
	);
}
