#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa::UDRefl;

namespace UInspector {
	struct Range {
		float min_value;
		float max_value;
	};
	class A {
	public:
		~A() {
			std::cout << "delete A" << std::endl;
		}
	};
}

struct Point {
	[[UInspector::range(1.f, 2.f)]]
	float x;
	[[UInspector::A]]
	float y;
};

int main() {
	auto ID_Point = ReflMngr::Instance().tregistry.Register("Point");
	auto ID_float = ReflMngr::Instance().tregistry.Register("float");
	auto ID_UInspector_Range = ReflMngr::Instance().tregistry.Register("UInspector::Range");
	auto ID_UInspector_A = ReflMngr::Instance().tregistry.Register("UInspector::A");

	auto ID_x = ReflMngr::Instance().nregistry.Register("x");
	auto ID_y = ReflMngr::Instance().nregistry.Register("y");
	auto ID_value = ReflMngr::Instance().nregistry.Register("value");
	auto ID_min_value = ReflMngr::Instance().nregistry.Register("min_value");
	auto ID_max_value = ReflMngr::Instance().nregistry.Register("max_value");

	{ // register range
		TypeInfo typeinfo{
			sizeof(UInspector::Range),
			alignof(UInspector::Range),
			{ // fields
				{ ID_min_value, { { ID_float, offsetof(UInspector::Range, min_value) } } },
				{ ID_max_value, { { ID_float, offsetof(UInspector::Range, max_value) } } },
			}
		};
		ReflMngr::Instance().typeinfos.emplace(ID_UInspector_Range, std::move(typeinfo));
	}

	{ // register Point
		TypeInfo typeinfo{
			sizeof(Point),
			alignof(Point),
			{ // fields
				{ ID_x, FieldInfo{ { ID_float, offsetof(Point, x) }, { // attrs
					{ID_UInspector_Range, MakeAttr<UInspector::Range>(1.f,2.f) }
				}}},
				{ ID_y, { { ID_float, offsetof(Point, y) } , { // attrs
					{ID_UInspector_A, MakeAttr<UInspector::Range>(1.f,2.f) }
				}}}
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
		[](TypeFieldInfo info, ConstObjectPtr field) {
			for (const auto& [attrID, attr] : info.fieldinfo.attrs) {
				std::cout << "[" << ReflMngr::Instance().tregistry.Nameof(attrID) << "]" << std::endl;
				ReflMngr::Instance().ForEachRField(
					attr.as_object(attrID),
					[](TypeFieldInfo info, ConstObjectPtr field) {
						std::cout
							<< ReflMngr::Instance().nregistry.Nameof(info.fieldID)
							<< ": " << field.As<float>()
							<< std::endl;
					}
				);
				std::cout << "------" << std::endl;
			}
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(info.fieldID)
				<< ": " << field.As<float>()
				<< std::endl;
		}
	);
}
