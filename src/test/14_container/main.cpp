#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

int main() {
	ReflMngr::Instance().RegisterTypeAuto<std::array<float, 5>>();
	SharedObject pArr = ReflMngr::Instance().MakeShared(TypeID::of<std::array<float, 5>>);
	for (auto ele : pArr.AsObjectPtr())
		std::cout << ele->TypeName() << ": " << ele->As<const float&>() << std::endl;
}
