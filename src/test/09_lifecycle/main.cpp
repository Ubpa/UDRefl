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
	p->RWVar("x") = 1.f;
	p->RWVar("y") = 2.f;

	for (const auto& [type, field, var] : p->GetTypeFieldRVars()) {
		std::cout
			<< ReflMngr::Instance().nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}
}
