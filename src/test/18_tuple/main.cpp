#include <UDRefl/UDRefl.hpp>
#include <iostream>
#include <tuple>

using namespace Ubpa;
using namespace Ubpa::UDRefl;


int main() {
	Mngr.RegisterType<std::tuple<int, float, double>>();

	for (const auto& [name, method] : MethodRange_of<std::tuple<int, float, double>>) {
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

		std::cout << " " << method.methodptr.GetResultType().GetName() << "(" ;

		for (const auto& param : method.methodptr.GetParamList())
			std::cout << param.GetName() << ", ";

		std::cout << ")" << std::endl;
	}

	SharedObject v = Mngr.MakeShared(Type_of<std::tuple<int, float, double>>, TempArgsView{ 3, 2, 1 });
	std::cout << v.GetType().GetName() << std::endl;

	for (std::size_t i{ 0 }; i < v.tuple_size(); i++)
		std::cout << v.tuple_element(i).GetName() << " (" << i << "): " << v.get(i) << std::endl;

	for (std::size_t i{ 0 }; i < v.tuple_size(); i++)
		v.get(i) = i;

	for (std::size_t i{ 0 }; i < v.tuple_size(); i++)
		std::cout << v.tuple_element(i).GetName() << " (" << i << "): " << v.get(i) << std::endl;

	v.get(Type_of<float>) = 512;

	for (std::size_t i{ 0 }; i < v.tuple_size(); i++)
		std::cout << v.tuple_element(i).GetName() << " (" << i << "): " << v.get(i) << std::endl;

	return 0;
}
