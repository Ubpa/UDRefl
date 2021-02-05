#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Point { float x, y; };

int main() {
	Mngr.RegisterType<Point>();
	Mngr.AddField<&Point::x>("x");
	Mngr.AddField<&Point::y>("y");
	
	SharedObject p = Mngr.MakeShared(TypeID_of<Point>);

	Mngr.Var(p, "x") = 1.f;
	Mngr.Var(p, "y") = 2.f;

	for (const auto& [type, field, var] : p.GetTypeFieldVars()) {
		std::cout
			<< Mngr.nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}

	const Point q{ 3.f,4.f };

	for (const auto& [type, field, var] : ObjectView{ q }.GetTypeFieldVars()) {
		std::cout
			<< Mngr.nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}
}
