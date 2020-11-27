#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa::UDRefl;

struct Point {
	[[UInspector::range(std::pair{0.f, 10.f})]]
	float x;
	float y;
};

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

	ReflMngr::Instance().ForEachRField(
		ptr,
		[](size_t typeID, const TypeInfo& typeinfo, size_t fieldID, const FieldInfo& fieldinfo, ConstObjectPtr field) {
			for (const auto& [attrID, attr] : fieldinfo.attrs) {
				if (attrID == ReflMngr::Instance().registry.GetID("UInspector_range")) {
					auto range = std::any_cast<std::pair<float, float>>(attr);
					std::cout
						<< "[UInspector_range]" << " "
						<< range.first << ", " << range.second
						<< std::endl;
				}
			}
			std::cout
				<< ReflMngr::Instance().registry.Nameof(fieldID)
				<< ": " << field.As<float>()
				<< std::endl;
		}
	);
}
