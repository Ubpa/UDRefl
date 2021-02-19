#include <UDRefl/UDRefl.h>
#include <iostream>
#include <variant>

using namespace Ubpa;
using namespace Ubpa::UDRefl;


int main() {
	Mngr->RegisterType<std::variant<int, float, double>>();

	for (auto&& method : Mngr->GetMethods(Type_of<std::variant<int, float, double>>)) {
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
		}
		std::cout << "]";

		std::cout << " " << method.info->methodptr.GetResultType().GetName() << "(";

		for (const auto& param : method.info->methodptr.GetParamList())
			std::cout << param.GetName() << ", ";

		std::cout << ")" << std::endl;
	}

	{
		SharedObject v = Mngr->MakeShared(Type_of<std::variant<int, float, double>>, 3.);
		std::cout << v.GetType().GetName() << std::endl;
		std::cout << v.variant_visit_get().GetType().GetName() << std::endl;

		std::cout << v.variant_index() << std::endl;
		std::cout << v.variant_holds_alternative(Type_of<double>) << std::endl;
		std::cout << v.variant_holds_alternative(Type_of<float>) << std::endl;

		for (size_t i = 0; i < v.variant_size(); ++i)
			std::cout << v.variant_alternative(i).GetName() << std::endl;
	}

	return 0;
}
