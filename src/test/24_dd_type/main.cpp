#include <UDRefl/UDRefl.hpp>
#include <iostream>
#include <cmath>

using namespace Ubpa;
using namespace Ubpa::UDRefl;


struct Empty {};

struct Base0 {
	int data0;
	Base0() : data0{ 1 } {
		std::cout << "ctor: Base0" << std::endl;
	}
	~Base0() {
		std::cout << "dtor: Base0" << std::endl;
	}
	friend std::ostream& operator<<(std::ostream& os, const Base0& b) {
		os << b.data0;
		return os;
	}
};

struct Base1 {
	double data1;
	~Base1() {
		std::cout << "dtor: Base1" << std::endl;
	}
	friend std::ostream& operator<<(std::ostream& os, const Base1& b) {
		os << b.data1;
		return os;
	}
};

struct Field2 {
	char data2;
	Field2() : data2{ 'c' } {
		std::cout << "ctor: Field2" << std::endl;
	}
	~Field2() {
		std::cout << "dtor: Field2" << std::endl;
	}
	friend std::ostream& operator<<(std::ostream& os, const Field2& b) {
		os << b.data2;
		return os;
	}
};

struct Field3 {
	int data3;
	friend std::ostream& operator<<(std::ostream& os, const Field3& b) {
		os << b.data3;
		return os;
	}
};

/*
struct A : Base0, Empty, Base1 {
    Field2 f2;
	Empty empty;
	Field3 f3;
};
*/

int main() {
	{
		Mngr.RegisterType<Empty>();
		Mngr.RegisterType<Base0>();
		Mngr.AddField<&Base0::data0>("data0");
		Mngr.RegisterType<Base1>();
		Mngr.AddField<&Base1::data1>("data1");
		Mngr.RegisterType<Field2>();
		Mngr.AddField<&Field2::data2>("data2");
		Mngr.RegisterType<Field3>();
		Mngr.AddField<&Field3::data3>("data3");

		Type bases[] = { Type_of<Base0>, Type_of<Empty>, Type_of<Base1> };
		Type field_types[] = { Type_of<Field2>, Type_of<Empty>, Type_of<Field3> };
		Name field_names[] = { Name{"f2"}, Name{"empty"}, Name{"f3"} };

		Mngr.RegisterType("A", bases, field_types, field_names);
		Mngr.AddDefaultConstructor("A");
		Mngr.AddDestructor("A");
	}

	for (auto&& [name, info] : FieldRange{ "A" })
		std::cout << name.GetView() << std::endl;

	for (const auto& [name, method] : MethodRange{ "A" }) {
		std::cout << name.GetView() << ": ";

		std::cout << "[";
		switch (method.methodptr.GetMethodFlag())
		{
		case MethodFlag::Variable:
			std::cout << "Variable";
			break;
		case MethodFlag::Const:
			std::cout << "Const";
			break;
		case MethodFlag::Static:
			std::cout << "Static";
			break;
		default:
			continue;
			break;
		}
		std::cout << "]";

		std::cout << " " << method.methodptr.GetResultType().GetName() << "(";

		for (const auto& param : method.methodptr.GetParamList())
			std::cout << param.GetName() << ", ";

		std::cout << ")" << std::endl;
	}

	SharedObject a = Mngr.MakeShared("A");
	a.Var("data1") = 1.23;
	a.Var("f3").Var("data3") = 1024;

	for (auto&& [name, var] : a.GetVars())
		std::cout << name.GetView() << ": " << var << std::endl;
}
