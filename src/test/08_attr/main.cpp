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
	{ // register
		ReflMngr::Instance().RegisterTypeAuto<UInspector::Range>();
		ReflMngr::Instance().AddField<&UInspector::Range::min_value>("min_value");
		ReflMngr::Instance().AddField<&UInspector::Range::max_value>("max_value");
		ReflMngr::Instance().AddConstructor<UInspector::Range, float, float>();

		ReflMngr::Instance().RegisterTypeAuto<UInspector::A>();

		ReflMngr::Instance().RegisterTypeAuto<Point>();
		ReflMngr::Instance().AddConstructor<Point, float, float>();
		ReflMngr::Instance().AddField<&Point::x>("x");
		ReflMngr::Instance().AddField<&Point::y>("y");
	}

	auto p = ReflMngr::Instance().MakeShared(TypeID::of<Point>, 1.f, 2.f);

	ReflMngr::Instance().RWVar(p, StrID{ "x" }).As<float>() += 1.f;
	ReflMngr::Instance().RWVar(p, StrID{ "y" }).As<float>() += 2.f;

	ReflMngr::Instance().ForEachRVar(
		p,
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			for (const auto& attr : field.info.attrs) {
				std::cout << "[" << ReflMngr::Instance().tregistry.Nameof(attr.GetID()) << "]" << std::endl;
				ReflMngr::Instance().ForEachRVar(
					attr,
					[](TypeRef type, FieldRef field, ConstObjectPtr var) {
						std::cout
							<< ReflMngr::Instance().nregistry.Nameof(field.ID)
							<< ": " << var.As<float>()
							<< std::endl;
						return true;
					}
				);
				std::cout << "------" << std::endl;
			}
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
			return true;
		}
	);
}
