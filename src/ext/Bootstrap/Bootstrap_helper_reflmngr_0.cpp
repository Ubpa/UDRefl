#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_reflmngr_0() {
	Mngr.RegisterType<ReflMngr>();
	Mngr.AddField<&ReflMngr::nregistry>("nregistry");
	Mngr.AddField<&ReflMngr::tregistry>("tregistry");
	Mngr.AddStaticMethod(Type_of<ReflMngr>, "Instance", &ReflMngr::Instance);
	Mngr.AddMethod<&ReflMngr::GetTypeInfo>("GetTypeInfo");
	Mngr.AddMethod<&ReflMngr::GetTypeAttr>("GetTypeAttr");
	Mngr.AddMethod<&ReflMngr::GetFieldAttr>("GetFieldAttr");
	Mngr.AddMethod<&ReflMngr::GetMethodAttr>("GetMethodAttr");
	Mngr.AddMethod<&ReflMngr::Clear>("Clear");
	Mngr.AddMethod<&ReflMngr::ContainsVirtualBase>("ContainsVirtualBase");
	Mngr.AddMethod<MemFuncOf<ReflMngr, Type(Type, std::size_t, std::size_t, bool, bool)>::get(&ReflMngr::RegisterType)>("RegisterType");
	Mngr.AddMemberMethod("RegisterType", [](ReflMngr& mngr, Type type, std::size_t size, std::size_t alignment) { return mngr.RegisterType(type, size, alignment); });
	Mngr.AddMemberMethod("RegisterType", [](ReflMngr& mngr, Type type, std::size_t size, std::size_t alignment, bool is_polymorphic) { return mngr.RegisterType(type, size, alignment, is_polymorphic); });
	Mngr.AddMethod<MemFuncOf<ReflMngr, Type(Type, std::span<const Type>, std::span<const Type>, std::span<const Name>, bool)>::get(&ReflMngr::RegisterType)>("RegisterType");
	Mngr.AddMemberMethod("RegisterType", [](ReflMngr& mngr, Type type, std::span<const Type> bases, std::span<const Type> field_types, std::span<const Name> field_names) { return mngr.RegisterType(type, bases, field_types, field_names); });
	Mngr.AddMethod<MemFuncOf<ReflMngr, Name(Type, Name, FieldInfo)>::get(&ReflMngr::AddField)>("AddField");
	Mngr.AddMethod<MemFuncOf<ReflMngr, Name(Type, Name, MethodInfo)>::get(&ReflMngr::AddMethod)>("AddMethod");
	Mngr.AddMethod<MemFuncOf<ReflMngr, Type(Type, Type, BaseInfo)>::get(&ReflMngr::AddBase)>("AddBase");
	Mngr.AddMethod<&ReflMngr::AddTypeAttr>("AddTypeAttr");
	Mngr.AddMethod<&ReflMngr::AddFieldAttr>("AddFieldAttr");
	Mngr.AddMethod<&ReflMngr::AddMethodAttr>("AddMethodAttr");
	Mngr.AddMethod<&ReflMngr::AddTrivialDefaultConstructor>("AddTrivialDefaultConstructor");
	Mngr.AddMethod<&ReflMngr::AddTrivialCopyConstructor>("AddTrivialCopyConstructor");
	Mngr.AddMethod<&ReflMngr::AddZeroDefaultConstructor>("AddZeroDefaultConstructor");
	Mngr.AddMethod<&ReflMngr::AddDefaultConstructor>("AddDefaultConstructor");
	Mngr.AddMethod<&ReflMngr::StaticCast_DerivedToBase>("StaticCast_DerivedToBase");
	Mngr.AddMethod<&ReflMngr::StaticCast_BaseToDerived>("StaticCast_BaseToDerived");
	Mngr.AddMethod<&ReflMngr::DynamicCast_BaseToDerived>("DynamicCast_BaseToDerived");
	Mngr.AddMethod<&ReflMngr::StaticCast>("StaticCast");
	Mngr.AddMethod<&ReflMngr::DynamicCast>("DynamicCast");
	Mngr.AddMethod<MemFuncOf<ReflMngr, ObjectView(ObjectView, Name, FieldFlag)const>::get(&ReflMngr::Var)>("Var");
	Mngr.AddMemberMethod("Var", [](ReflMngr& mngr, ObjectView obj, Name field_name) {return mngr.Var(obj, field_name); });
	Mngr.AddMethod<MemFuncOf<ReflMngr, ObjectView(ObjectView, Type, Name, FieldFlag)const>::get(&ReflMngr::Var)>("Var");
	Mngr.AddMemberMethod("Var", [](ReflMngr& mngr, ObjectView obj, Type base, Name field_name) {return mngr.Var(obj, base, field_name); });
}
