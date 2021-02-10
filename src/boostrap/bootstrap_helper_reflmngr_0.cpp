#include "bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::bootstrap_helper_reflmngr_0() {
	Mngr.RegisterType<ReflMngr>();
	Mngr.AddField<&ReflMngr::nregistry>("nregistry");
	Mngr.AddField<&ReflMngr::tregistry>("tregistry");
	Mngr.AddStaticMethod(Type_of<ReflMngr>, "Instance", &ReflMngr::Instance);
	Mngr.AddMethod<MemFuncOf<ReflMngr, TypeInfo* (Type)>::get(&ReflMngr::GetTypeInfo)>("GetTypeInfo");
	Mngr.AddMethod<MemFuncOf<ReflMngr, const TypeInfo* (Type) const>::get(&ReflMngr::GetTypeInfo)>("GetTypeInfo");
	Mngr.AddMethod<&ReflMngr::Clear>("Clear");
	Mngr.AddMethod<MemFuncOf<ReflMngr, Type(Type, std::size_t, std::size_t)>::get(&ReflMngr::RegisterType)>("RegisterType");
	Mngr.AddMethod<MemFuncOf<ReflMngr, Name(Type, Name, FieldInfo)>::get(&ReflMngr::AddField)>("AddField");
	Mngr.AddMethod<MemFuncOf<ReflMngr, Name(Type, Name, MethodInfo)>::get(&ReflMngr::AddMethod)>("AddField");
	Mngr.AddMethod<MemFuncOf<ReflMngr, Type(Type, Type, BaseInfo)>::get(&ReflMngr::AddBase)>("AddField");
	Mngr.AddMethod<&ReflMngr::AddAttr>("AddAttr");
	Mngr.AddMethod<&ReflMngr::StaticCast_DerivedToBase>("StaticCast_DerivedToBase");
	Mngr.AddMethod<&ReflMngr::StaticCast_BaseToDerived>("StaticCast_BaseToDerived");
	Mngr.AddMethod<&ReflMngr::DynamicCast_BaseToDerived>("DynamicCast_BaseToDerived");
	Mngr.AddMethod<&ReflMngr::StaticCast>("StaticCast");
	Mngr.AddMethod<&ReflMngr::DynamicCast>("DynamicCast");
	Mngr.AddMethod<MemFuncOf<ReflMngr, ObjectView(ObjectView, Name, FieldFlag)>::get(&ReflMngr::Var)>("Var");
	Mngr.AddMethod<MemFuncOf<ReflMngr, ObjectView(ObjectView, Type, Name, FieldFlag)>::get(&ReflMngr::Var)>("Var");
	Mngr.AddMethod<MemFuncOf<ReflMngr, ObjectView(ObjectView, Type, Name, FieldFlag)>::get(&ReflMngr::Var)>("Var");

	Mngr.AddMethod<&ReflMngr::GetTypes>("GetTypes");
	Mngr.AddMethod<&ReflMngr::GetTypeFields>("GetTypeFields");
	Mngr.AddMethod<&ReflMngr::GetFields>("GetFields");
	Mngr.AddMethod<&ReflMngr::GetTypeMethods>("GetTypeMethods");
	Mngr.AddMethod<&ReflMngr::GetMethods>("GetMethods");
	Mngr.AddMethod<&ReflMngr::GetTypeFieldVars>("GetTypeFieldVars");
	Mngr.AddMethod<&ReflMngr::GetVars>("GetVars");

	Mngr.AddMethod<&ReflMngr::ContainsBase>("ContainsBase");
	Mngr.AddMethod<&ReflMngr::ContainsField>("ContainsField");
	Mngr.AddMethod<&ReflMngr::ContainsMethod>("ContainsMethod");
}
