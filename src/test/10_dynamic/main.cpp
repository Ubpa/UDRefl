#include <UDRefl/UDRefl.hpp>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

enum class Color {
	Red,
	Green,
	Blue
};

int main() {
	Mngr.AddDynamicField<const std::string>(GlobalType, "author", "Ubpa");
	Mngr.AddDynamicField<const Color>(GlobalType, "theme", Color::Red);
	Mngr.AddDynamicField<const size_t>(Type_of<int>, "bits", sizeof(int) * 8);

	std::cout << Global.Var("author") << std::endl;
	std::cout << (Global.Var("theme") == Color::Red) << std::endl;

	std::cout << ObjectView_of<int>.Var("bits") << std::endl;

	return 0;
}
