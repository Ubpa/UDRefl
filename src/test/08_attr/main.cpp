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
	[[UInspector::Range(1.f, 2.f)]]
	float x;
	[[UInspector::A]]
	float y;
};

int main() {
	{ // register
		Mngr->RegisterTypeAuto<UInspector::Range>();
		Mngr->AddField<&UInspector::Range::min_value>("min_value");
		Mngr->AddField<&UInspector::Range::max_value>("max_value");
		Mngr->AddConstructor<UInspector::Range, float, float>();

		Mngr->RegisterTypeAuto<UInspector::A>();

		Mngr->RegisterTypeAuto<Point>();
		Mngr->AddConstructor<Point, float, float>();
		Mngr->AddField<&Point::x>("x", { Mngr->MakeShared(TypeID_of<UInspector::Range>, 1.f, 2.f) });
		Mngr->AddField<&Point::y>("y", { Mngr->MakeShared(TypeID_of<UInspector::A>) });
	}

	auto p = Mngr->MakeShared(TypeID_of<Point>, 1.f, 2.f);

	Mngr->RWVar(p, StrID{ "x" }).As<float>() += 1.f;
	Mngr->RWVar(p, StrID{ "y" }).As<float>() += 2.f;

	p->ForEachRVar(
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			for (const auto& attr : field.info.attrs) {
				std::cout << "[" << Mngr->tregistry.Nameof(attr.GetID()) << "]" << std::endl;
				attr->ForEachRVar(
					[](TypeRef type, FieldRef field, ConstObjectPtr var) {
						std::cout
							<< Mngr->nregistry.Nameof(field.ID)
							<< ": " << var.As<float>()
							<< std::endl;
						return true;
					}
				);
				std::cout << "------" << std::endl;
			}
			std::cout
				<< Mngr->nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
			return true;
		}
	);
}
