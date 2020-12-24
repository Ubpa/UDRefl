#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Point {
	Point() {
		std::cout << "Point ctor" << std::endl;
	}
	~Point() {
		std::cout << "Point dtor" << std::endl;
	}
	float x, y;
};

int main() {
	ReflMngr::Instance().RegisterTypeAuto<Point>();
	ReflMngr::Instance().AddField<&Point::x>("x");
	ReflMngr::Instance().AddField<&Point::y>("y");
	
	SharedObject p = ReflMngr::Instance().MakeShared(TypeID_of<Point>);
	ReflMngr::Instance().RWVar(p, StrID{ "x" }) = 1.f;
	ReflMngr::Instance().RWVar(p, StrID{ "y" }) = 2.f;

	p->ForEachRVar(
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var
				<< std::endl;
			return true;
		}
	);
}
