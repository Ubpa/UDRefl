#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

int main() {
	ReflMngr::Instance().AddDynamicField<const std::string>(GlobalID, "author", "Ubpa");
	ReflMngr::Instance().RegisterTypePro<int>();
	ReflMngr::Instance().AddDynamicField<const size_t>(TypeID::of<int>, "bits", sizeof(int) * 8);

	std::cout << ReflMngr::Instance().RVar(GlobalID, StrID{ "author" }).As<std::string>() << std::endl;

	std::cout << ReflMngr::Instance().RVar(TypeID::of<int>, StrID{ "bits" }).As<size_t>() << std::endl;
}
