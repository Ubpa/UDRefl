#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa::UDRefl;

struct Point {
	float x;
	float y;
};

int main() {
	auto ID_Point = ReflMngr::Instance().tregistry.Register("Point");
	auto ID_float = ReflMngr::Instance().tregistry.Register("float");

	auto ID_x = ReflMngr::Instance().nregistry.Register("x");
	auto ID_y = ReflMngr::Instance().nregistry.Register("y");

	{ // register Point
		TypeInfo typeinfo{{
			{ ID_x, { { ID_float, offsetof(Point, x) } }},
			{ ID_y, { { ID_float, offsetof(Point, y) } }}
		}};
		ReflMngr::Instance().typeinfos.emplace(ID_Point, std::move(typeinfo));
	}
	
	Point p;
	ObjectPtr ptr{ ID_Point, &p };
	ReflMngr::Instance().RWField(ptr, ID_x).As<float>() = 1.f;
	ReflMngr::Instance().RWField(ptr, ID_y).As<float>() = 2.f;

	ReflMngr::Instance().ForEachRField(
		ptr,
		[](TypeFieldInfo info, ConstObjectPtr field) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(info.fieldID)
				<< ": " << field.As<float>()
				<< std::endl;
		}
	);
}
