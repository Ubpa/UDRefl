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
	auto ID_Point = ReflMngr::Instance().tregistry.GetID<Point>();
	auto ID_float = ReflMngr::Instance().tregistry.GetID<float>();
	auto ID_UInspector_Range = ReflMngr::Instance().tregistry.GetID<UInspector::Range>();
	auto ID_UInspector_A = ReflMngr::Instance().tregistry.GetID<UInspector::A>();

	auto ID_x = ReflMngr::Instance().nregistry.GetID("x");
	auto ID_y = ReflMngr::Instance().nregistry.GetID("y");
	auto ID_value = ReflMngr::Instance().nregistry.GetID("value");
	auto ID_min_value = ReflMngr::Instance().nregistry.GetID("min_value");
	auto ID_max_value = ReflMngr::Instance().nregistry.GetID("max_value");

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
					{ID_UInspector_Range, MakeSharedBlock<UInspector::Range>(1.f,2.f) }
				}}},
				{ ID_y, { { ID_float, offsetof(Point, y) } , { // attrs
					{ID_UInspector_A, MakeSharedBlock<UInspector::Range>(1.f,2.f) }
				}}}
			}
		};
		ReflMngr::Instance().typeinfos.emplace(ID_Point, std::move(typeinfo));
	}

	Point p;
	ObjectPtr ptr{ ID_Point, &p };
	ReflMngr::Instance().RWVar(ptr, ID_x).As<float>() = 1.f;
	ReflMngr::Instance().RWVar(ptr, ID_y).As<float>() = 2.f;

	ReflMngr::Instance().ForEachRVar(
		ptr,
		[](Type type, Field field, ConstObjectPtr var) {
			for (const auto& [attrID, attr] : field.info.attrs) {
				std::cout << "[" << ReflMngr::Instance().tregistry.Nameof(attrID) << "]" << std::endl;
				ReflMngr::Instance().ForEachRVar(
					attr.AsObjectPtr(attrID),
					[](Type type, Field field, ConstObjectPtr var) {
						std::cout
							<< ReflMngr::Instance().nregistry.Nameof(field.ID)
							<< ": " << var.As<float>()
							<< std::endl;
					}
				);
				std::cout << "------" << std::endl;
			}
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
		}
	);
}
