#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa;
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
	auto ID_Point = ReflMngr::Instance().tregistry.Register<Point>();
	auto ID_float = ReflMngr::Instance().tregistry.Register<float>();
	auto ID_UInspector_Range = ReflMngr::Instance().tregistry.Register<UInspector::Range>();
	auto ID_UInspector_A = ReflMngr::Instance().tregistry.Register<UInspector::A>();

	auto ID_value = ReflMngr::Instance().nregistry.Register("value");
	auto ID_min_value = ReflMngr::Instance().nregistry.Register("min_value");
	auto ID_max_value = ReflMngr::Instance().nregistry.Register("max_value");

	{ // register UInspector::Range
		TypeInfo typeinfo{
			sizeof(UInspector::Range),
			alignof(UInspector::Range),
			{ // fields
				ReflMngr::Instance().GenerateField<&UInspector::Range::min_value>("min_value"),
				ReflMngr::Instance().GenerateField<&UInspector::Range::max_value>("max_value"),
			},
			{ // methods
				ReflMngr::Instance().GenerateConstructor<UInspector::Range>(),
				ReflMngr::Instance().GenerateConstructor<UInspector::Range, float, float>(),
				ReflMngr::Instance().GenerateDestructor<UInspector::Range>()
			}
		};
		ReflMngr::Instance().typeinfos.emplace(ID_UInspector_Range, std::move(typeinfo));
	}

	{  // register UInspector::A
		TypeInfo typeinfo{
			sizeof(UInspector::A),
			alignof(UInspector::A),
			{}, // fields
			{ // methods
				ReflMngr::Instance().GenerateConstructor<UInspector::A>(),
				ReflMngr::Instance().GenerateDestructor<UInspector::A>()
			}
		};
		ReflMngr::Instance().typeinfos.emplace(ID_UInspector_A, std::move(typeinfo));
	}
	{ // register Point
		TypeInfo typeinfo{
			sizeof(Point),
			alignof(Point),
			{ // fields
				ReflMngr::Instance().GenerateField<&Point::x>("x", {ReflMngr::Instance().MakeShared<UInspector::Range>(1.f,2.f)}),
				ReflMngr::Instance().GenerateField<&Point::y>("y", {ReflMngr::Instance().MakeShared<UInspector::A>()})
			}
		};
		ReflMngr::Instance().typeinfos.emplace(ID_Point, std::move(typeinfo));
	}

	Point p;
	ObjectPtr ptr{ ID_Point, &p };
	ReflMngr::Instance().RWVar(ptr, StrID{ "x" }).As<float>() = 1.f;
	ReflMngr::Instance().RWVar(ptr, StrID{ "y" }).As<float>() = 2.f;

	ReflMngr::Instance().ForEachRVar(
		ptr,
		[](Type type, Field field, ConstObjectPtr var) {
			for (const auto& attr : field.info.attrs) {
				std::cout << "[" << ReflMngr::Instance().tregistry.Nameof(attr.GetID()) << "]" << std::endl;
				ReflMngr::Instance().ForEachRVar(
					attr,
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
