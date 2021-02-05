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
	Mngr.RegisterType<Point>();
	Mngr.AddField<&Point::x>("x");
	Mngr.AddField<&Point::y>("y");
	
	SharedObject p = Mngr.MakeShared(TypeID_of<Point>);
	p.Var("x") = 1.f;
	p.Var("y") = 2.f;

	for (const auto& [type, field, var] : p.GetTypeFieldVars()) {
		std::cout
			<< Mngr.nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}
}
