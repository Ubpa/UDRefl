#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Point { float x, y; };

int main() {
	ReflMngr::Instance().RegisterType<Point>();
	ReflMngr::Instance().AddField<&Point::x>("x");
	ReflMngr::Instance().AddField<&Point::y>("y");
	
	SharedObject p = ReflMngr::Instance().MakeShared(TypeID_of<Point>);

	ReflMngr::Instance().Var(p, "x") = 1.f;
	ReflMngr::Instance().Var(p, "y") = 2.f;

	for (const auto& [type, field, var] : p->GetTypeFieldVars()) {
		std::cout
			<< ReflMngr::Instance().nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}

	const Point q{ 3.f,4.f };

	for (const auto& [type, field, var] : ObjectPtr{ q }->GetTypeFieldVars()) {
		std::cout
			<< ReflMngr::Instance().nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}
}
