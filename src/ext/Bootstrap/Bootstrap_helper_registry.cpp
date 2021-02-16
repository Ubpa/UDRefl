#include "Bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

template<typename T, typename U>
void RegisterIDRegistry() {
	using W = IDRegistry<T, U>;
	Mngr->RegisterType<W>();
	Mngr->AddMethod<MemFuncOf<W, void(T, std::string_view)>::get(&W::RegisterUnmanaged)>("RegisterUnmanaged");
	Mngr->AddMethod<MemFuncOf<W, T(std::string_view)>::get(&W::RegisterUnmanaged)>("RegisterUnmanaged");
	Mngr->AddMethod<MemFuncOf<W, std::string_view(T, std::string_view)>::get(&W::Register)>("Register");
	Mngr->AddMethod<MemFuncOf<W, U(std::string_view)>::get(&W::Register)>("Register");
	Mngr->AddMethod<&W::IsRegistered>("IsRegistered");
	Mngr->AddMethod<&W::Nameof>("Nameof");
	Mngr->AddMethod<&W::UnregisterUnmanaged>("UnregisterUnmanaged");
	Mngr->AddMethod<&W::Clear>("Clear");
}

void Ubpa::UDRefl::ext::details::Bootstrap_helper_registry() {
	RegisterIDRegistry<NameID, Name>();
	Mngr->RegisterType<NameIDRegistry>();
	Mngr->AddBases<NameIDRegistry, IDRegistry<NameID, Name>>();

	RegisterIDRegistry<TypeID, Type>();
	Mngr->RegisterType<TypeIDRegistry>();
	Mngr->AddBases<TypeIDRegistry, IDRegistry<TypeID, Type>>();
	Mngr->AddMethod<&TypeIDRegistry::RegisterAddConst>("RegisterAddConst");
	Mngr->AddMethod<&TypeIDRegistry::RegisterAddLValueReference>("RegisterAddLValueReference");
	Mngr->AddMethod<&TypeIDRegistry::RegisterAddLValueReferenceWeak>("RegisterAddLValueReferenceWeak");
	Mngr->AddMethod<&TypeIDRegistry::RegisterAddRValueReference>("RegisterAddRValueReference");
	Mngr->AddMethod<&TypeIDRegistry::RegisterAddConstLValueReference>("RegisterAddConstLValueReference");
	Mngr->AddMethod<&TypeIDRegistry::RegisterAddConstRValueReference>("RegisterAddConstRValueReference");
}
