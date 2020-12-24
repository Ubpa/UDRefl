#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

enum class Color {
	Red,
	Green,
	Blue
};

int main() {
	ReflMngr::Instance().AddDynamicField<const std::string>(GlobalID, "author", "Ubpa");
	ReflMngr::Instance().AddDynamicField<const Color>(GlobalID, "theme", Color::Red);
	ReflMngr::Instance().RegisterTypeAuto<int>();
	ReflMngr::Instance().AddDynamicField<const size_t>(TypeID_of<int>, "bits", sizeof(int) * 8);

	std::cout << ReflMngr::Instance().RVar(GlobalID, "author") << std::endl;
	std::cout << (ReflMngr::Instance().RVar(GlobalID, "theme").As<Color>() == Color::Red) << std::endl;

	std::cout << ReflMngr::Instance().RVar(TypeID_of<int>, "bits") << std::endl;
}
