#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

int main() {
	auto& global = ReflMngr::Instance().typeinfos.at(TypeID{TypeIDRegistry::Meta::global});
	SharedBlock block = MakeSharedBlock<std::string>("Ubpa");
	global.fieldinfos.emplace(ReflMngr::Instance().GenerateDynamicField<const std::string>("author", "Ubpa"));
	std::cout << ReflMngr::Instance().RVar(StrID{ "author" }).As<std::string>() << std::endl;
}
