#include <UDRefl/UDRefl.h>
#include <iostream>
#include <cmath>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

int main() {
	std::cout
		<< "//" << std::endl
		<< "// std::string" << std::endl
		<< "////////////////" << std::endl;
	for (auto&& method : Mngr.GetMethods(Type_of<std::string>))
		std::cout << method.name.GetView() << std::endl;
	std::cout
		<< "//" << std::endl
		<< "// std::string_view" << std::endl
		<< "/////////////////////" << std::endl;
	for (auto&& method : Mngr.GetMethods(Type_of<std::string_view>))
		std::cout << method.name.GetView() << std::endl;
	std::cout
		<< "//" << std::endl
		<< "// const char*" << std::endl
		<< "/////////////////////" << std::endl;
	for (auto&& method : Mngr.GetMethods(Type_of<const char*>))
		std::cout << method.name.GetView() << std::endl;

	auto str = Mngr.MakeShared(Type_of<std::string>, TempArgsView{ "hello world" });
	
	std::cout << str << std::endl;

	return 0;
}
