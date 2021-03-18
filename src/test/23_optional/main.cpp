#include <UDRefl/UDRefl.hpp>
#include <iostream>
#include <optional>

using namespace Ubpa;
using namespace Ubpa::UDRefl;


int main() {
	Mngr.RegisterType<std::optional<float>>();

	for (const auto& [name, method] : MethodRange_of<std::optional<float>>) {
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

	SharedObject v = Mngr.MakeShared(Type_of<std::optional<float>>, TempArgsView{ 3 });
	std::cout << v.GetType().GetName() << std::endl;

	std::cout << v.has_value() << std::endl;
	std::cout << v.value() << std::endl;
	v.reset();
	std::cout << v.has_value() << std::endl;

	return 0;
}
