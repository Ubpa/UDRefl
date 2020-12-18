#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

int main() {
	ReflMngr::Instance().RegisterTypeAuto<std::array<size_t, 5>>();

	for (const auto& [ID, info] : ReflMngr::Instance().typeinfos) {
		ReflMngr::Instance().ForEachMethod(ID, [](TypeRef type, MethodRef method) {
			std::cout << ReflMngr::Instance().tregistry.Nameof(type.ID) << ": "
				<< ReflMngr::Instance().nregistry.Nameof(method.ID)
				<< std::endl;
			return true;
		});
	}

	SharedObject arr = ReflMngr::Instance().MakeShared(TypeID::of<std::array<size_t, 5>>);
	for (size_t i = 0; i < arr->size(); i++)
		arr[i] = i;
	for (SharedObject ele : arr)
		std::cout << ele->TypeName() << ": " << ele << std::endl;
}
