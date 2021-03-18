#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

template<typename T, typename U>
void RegisterIDRegistry() {
	using W = IDRegistry<T, U>;
	Mngr.RegisterType<W>();
	Mngr.AddMethod<MemFuncOf<W, std::string_view(T, std::string_view)>::get(&W::Register)>("Register");
	Mngr.AddMethod<MemFuncOf<W, U(std::string_view)>::get(&W::Register)>("Register");
	Mngr.AddMethod<&W::IsRegistered>("IsRegistered");
	Mngr.AddMethod<&W::Viewof>("Viewof");
	Mngr.AddMethod<&W::Clear>("Clear");
}

void Ubpa::UDRefl::ext::details::Bootstrap_helper_registry() {
	RegisterIDRegistry<NameID, Name>();
	Mngr.RegisterType<NameIDRegistry>();
	Mngr.AddMethod<MemFuncOf<NameIDRegistry, Name(Name)>::get(&NameIDRegistry::Register)>("Register");
	Mngr.AddMethod<&NameIDRegistry::Nameof>("Nameof");
	Mngr.AddBases<NameIDRegistry, IDRegistry<NameID, Name>>();

	RegisterIDRegistry<TypeID, Type>();
	Mngr.RegisterType<TypeIDRegistry>();
	Mngr.AddBases<TypeIDRegistry, IDRegistry<TypeID, Type>>();
	Mngr.AddMethod<MemFuncOf<TypeIDRegistry, Type(Type)>::get(&TypeIDRegistry::Register)>("Register");
	Mngr.AddMethod<&TypeIDRegistry::Typeof>("Typeof");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddConst>("RegisterAddConst");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddLValueReference>("RegisterAddLValueReference");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddLValueReferenceWeak>("RegisterAddLValueReferenceWeak");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddRValueReference>("RegisterAddRValueReference");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddConstLValueReference>("RegisterAddConstLValueReference");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddConstRValueReference>("RegisterAddConstRValueReference");
}
