#include <UDRefl/UDRefl.h>

#include <iostream>
#include <cassert>

using namespace Ubpa::UDRefl;

struct Point {
	[[UInspector::range(std::pair{0.f, 10.f})]]
	float x;
	float y;
};

struct A { virtual ~A() {}; };
struct B : virtual A {};
struct C : virtual A {};
struct D : B, C {};

int main() {
	size_t ID_Point = ReflMngr::Instance().registry.Register("Point");
	size_t ID_float = ReflMngr::Instance().registry.Register("float");
	size_t ID_x = ReflMngr::Instance().registry.Register("x");
	size_t ID_y = ReflMngr::Instance().registry.Register("y");
	size_t ID_UInspector_range = ReflMngr::Instance().registry.Register("UInspector_range");

	{ // register Point
		TypeInfo typeinfo{
			{}, // attrs
			{ // fields
				{ ID_x, { { ID_float, offsetof(Point, x) }, { // attrs
					{ID_UInspector_range, std::pair{0.f, 10.f}}
				}}},
				{ ID_y, { { ID_float, offsetof(Point, y) } }}
			}
		};
		ReflMngr::Instance().typeinfos.emplace(ID_Point, std::move(typeinfo));
	}
	
	Point p;
	ObjectPtr ptr{ ID_Point, &p };
	ReflMngr::Instance().RWField(ptr, ID_x).As<float>() = 1.f;
	ReflMngr::Instance().RWField(ptr, ID_y).As<float>() = 2.f;

	for (const auto& [ID_field, fieldinfo] : ReflMngr::Instance().typeinfos.at(ID_Point).fieldinfos) {
		auto field_name = ReflMngr::Instance().registry.Nameof(ID_field);
		if (fieldinfo.fieldptr.GetValueID() == ID_float) {
			std::cout << field_name << std::endl;
			if (fieldinfo.attrs.find(ID_UInspector_range) != fieldinfo.attrs.end()) {
				const auto& r = std::any_cast<const std::pair<float, float>&>(fieldinfo.attrs.at(ID_UInspector_range));
				std::cout << ReflMngr::Instance().registry.Nameof(ID_UInspector_range) << ": " << r.first << ", " << r.second << std::endl;
			}
		}
	}
}
