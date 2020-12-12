#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Point { float x, y; };

int main() {
	ReflMngr::Instance().RegisterTypeAuto<Point>();
	ReflMngr::Instance().AddField<&Point::x>("x");
	ReflMngr::Instance().AddField<&Point::x>("y");
	
	Point p;
	ObjectPtr ptr{ TypeID::of<Point>, &p };
	ReflMngr::Instance().RWVar(ptr, StrID{ "x" }).As<float>() = 1.f;
	ReflMngr::Instance().RWVar(ptr, StrID{ "y" }).As<float>() = 2.f;

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
