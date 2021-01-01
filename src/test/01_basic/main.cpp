#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Point { float x, y; };

int main() {
	ReflMngr::Instance().RegisterTypeAuto<Point>();
	ReflMngr::Instance().AddField<&Point::x>("x");
	ReflMngr::Instance().AddField<&Point::y>("y");
	
	SharedObject p = ReflMngr::Instance().MakeShared(TypeID_of<Point>);

	ReflMngr::Instance().RWVar(p, "x") = 1.f;
	ReflMngr::Instance().RWVar(p, "y") = 2.f;

	for (const auto& [type, field, var] : p->GetTypeFieldRVars()) {
		std::cout
			<< ReflMngr::Instance().nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}

	const Point q{ 3.f,4.f };

	for (const auto& [type, field, var] : Ptr(q)->GetTypeFieldRVars()) {
		std::cout
			<< ReflMngr::Instance().nregistry.Nameof(field.ID)
			<< ": " << var
			<< std::endl;
	}
}
