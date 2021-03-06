#include <UDRefl/UDRefl.h>
#include <iostream>
#include <optional>

using namespace Ubpa;
using namespace Ubpa::UDRefl;


int main() {
	Mngr.RegisterType<std::optional<float>>();
	for (auto&& method : Mngr.GetMethods(Type_of<std::optional<float>>)) {
		std::cout << method.name.GetView() << ": ";

		std::cout << "[";
		switch (method.info->methodptr.GetMethodFlag())
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
		}
		std::cout << "]";

		std::cout << " " << method.info->methodptr.GetResultType().GetName() << "(";

		for (const auto& param : method.info->methodptr.GetParamList())
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
