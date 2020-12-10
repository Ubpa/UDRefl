#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa::UDRefl;

int main() {
	auto& global = ReflMngr::Instance().typeinfos.at(TypeRegistry::DirectGetID(TypeRegistry::Meta::global));
	SharedBlock block = MakeSharedBlock<std::string>("Ubpa");
	global.fieldinfos.emplace(
		ReflMngr::Instance().nregistry.GetID("author"),
		FieldInfo{ FieldPtr{ SharedObject{ ReflMngr::Instance().tregistry.GetID<std::string>(), std::move(block) } } }
	);
	std::cout << ReflMngr::Instance().RVar(ReflMngr::Instance().nregistry.DirectGetID("author")).As<std::string>() << std::endl;
}
