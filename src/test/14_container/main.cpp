#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

int main() {
	ReflMngr::Instance().RegisterTypeAuto<std::array<float, 5>>();

	SharedObject arr = ReflMngr::Instance().MakeShared(TypeID::of<std::array<float, 5>>);
	for (SharedObject ele : arr)
		std::cout << ele->TypeName() << ": " << ele->As<float&>() << std::endl;
}
