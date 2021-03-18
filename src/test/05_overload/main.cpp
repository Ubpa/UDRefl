#include <UDRefl/UDRefl.hpp>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Vec {
	float x;
	float y;

	Vec& operator+=(const Vec& p) {
		std::cout << "Vec& operator+=(const Vec&)" << std::endl;
		x += p.x;
		y += p.y;
		return *this;
	}

	Vec& operator+=(float d) {
		std::cout << "Vec& operator+=(float)" << std::endl;
		x += d;
		y += d;
		return *this;
	}
};

int main() {
	{ // register Vec
		Mngr.RegisterType<Vec>();
		Mngr.AddField<&Vec::x>("x");
		Mngr.AddField<&Vec::y>("y");
		Mngr.AddMethod<MemFuncOf<Vec, Vec& (float)>::get(&Vec::operator+=)>(NameIDRegistry::Meta::operator_assignment_add);
	}
	
	auto obj = Mngr.MakeShared(Type_of<Vec>);
	obj.Var("x") = 3;
	obj.Var("y") = 4;

	{
		auto v = obj += Vec{ 10.f,10.f };
		std::cout << v.Var("x") << ", " << v.Var("y") << std::endl;
	}

	{
		auto v = obj += 2.f;
		std::cout << v.Var("x") << ", " << v.Var("y") << std::endl;
	}
	
	return 0;
}
