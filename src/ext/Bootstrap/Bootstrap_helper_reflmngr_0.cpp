#include "Bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_reflmngr_0() {
	Mngr.RegisterType<ReflMngr>();
	Mngr.AddField<&ReflMngr::nregistry>("nregistry");
	Mngr.AddField<&ReflMngr::tregistry>("tregistry");
	Mngr.AddStaticMethod(Type_of<ReflMngr>, "Instance", &ReflMngr::Instance);
	Mngr.AddMethod<MemFuncOf<ReflMngr, TypeInfo* (Type)>::get(&ReflMngr::GetTypeInfo)>("GetTypeInfo");
	Mngr.AddMethod<&ReflMngr::Clear>("Clear");
	Mngr.AddMethod<MemFuncOf<ReflMngr, Type(Type, std::size_t, std::size_t)>::get(&ReflMngr::RegisterType)>("RegisterType");
	Mngr.AddMethod<MemFuncOf<ReflMngr, Name(Type, Name, FieldInfo)>::get(&ReflMngr::AddField)>("AddField");
	Mngr.AddMethod<MemFuncOf<ReflMngr, Name(Type, Name, MethodInfo)>::get(&ReflMngr::AddMethod)>("AddMethod");
	Mngr.AddMethod<MemFuncOf<ReflMngr, Type(Type, Type, BaseInfo)>::get(&ReflMngr::AddBase)>("AddBase");
	Mngr.AddMethod<&ReflMngr::AddTypeAttr>("AddTypeAttr");
	Mngr.AddMethod<&ReflMngr::AddFieldAttr>("AddFieldAttr");
	Mngr.AddMethod<&ReflMngr::AddMethodAttr>("AddMethodAttr");
	Mngr.AddMethod<&ReflMngr::AddTrivialConstructor>("AddTrivialConstructor");
	Mngr.AddMethod<&ReflMngr::AddZeroConstructor>("AddZeroConstructor");
	Mngr.AddMethod<&ReflMngr::StaticCast_DerivedToBase>("StaticCast_DerivedToBase");
	Mngr.AddMethod<&ReflMngr::StaticCast_BaseToDerived>("StaticCast_BaseToDerived");
	Mngr.AddMethod<&ReflMngr::DynamicCast_BaseToDerived>("DynamicCast_BaseToDerived");
	Mngr.AddMethod<&ReflMngr::StaticCast>("StaticCast");
	Mngr.AddMethod<&ReflMngr::DynamicCast>("DynamicCast");
	Mngr.AddMethod<MemFuncOf<ReflMngr, ObjectView(ObjectView, Name, FieldFlag)>::get(&ReflMngr::Var)>("Var");
	Mngr.AddMemberMethod("Var", [](ReflMngr& mngr, ObjectView obj, Name field_name) {return mngr.Var(obj, field_name); });
	Mngr.AddMethod<MemFuncOf<ReflMngr, ObjectView(ObjectView, Type, Name, FieldFlag)>::get(&ReflMngr::Var)>("Var");
	Mngr.AddMemberMethod("Var", [](ReflMngr& mngr, ObjectView obj, Type base, Name field_name) {return mngr.Var(obj, base, field_name); });

	Mngr.AddMethod<&ReflMngr::GetTypes>("GetTypes");
	Mngr.AddMethod<&ReflMngr::GetTypeFields>("GetTypeFields");
	Mngr.AddMethod<&ReflMngr::GetFields>("GetFields");
	Mngr.AddMethod<&ReflMngr::GetTypeMethods>("GetTypeMethods");
	Mngr.AddMethod<&ReflMngr::GetMethods>("GetMethods");
	Mngr.AddMethod<&ReflMngr::GetTypeFieldVars>("GetTypeFieldVars");
	Mngr.AddMethod<&ReflMngr::GetVars>("GetVars");
	Mngr.AddMemberMethod("GetTypeFields", [](ReflMngr& mngr, Type type) {return mngr.GetTypeFields(type); });
	Mngr.AddMethod<&ReflMngr::GetFields>("GetFields");
	Mngr.AddMemberMethod("GetFields", [](ReflMngr& mngr, Type type) {return mngr.GetFields(type); });
	Mngr.AddMethod<&ReflMngr::GetTypeMethods>("GetTypeMethods");
	Mngr.AddMemberMethod("GetTypeMethods", [](ReflMngr& mngr, Type type) {return mngr.GetTypeMethods(type); });
	Mngr.AddMethod<&ReflMngr::GetMethods>("GetMethods");
	Mngr.AddMemberMethod("GetMethods", [](ReflMngr& mngr, Type type) {return mngr.GetMethods(type); });
	Mngr.AddMethod<&ReflMngr::GetTypeFieldVars>("GetTypeFieldVars");
	Mngr.AddMemberMethod("GetTypeFieldVars", [](ReflMngr& mngr, ObjectView obj) {return mngr.GetTypeFieldVars(obj); });
	Mngr.AddMethod<&ReflMngr::GetVars>("GetVars");
	Mngr.AddMemberMethod("GetVars", [](ReflMngr& mngr, ObjectView obj) {return mngr.GetVars(obj); });

	Mngr.AddMethod<&ReflMngr::ContainsBase>("ContainsBase");
	Mngr.AddMemberMethod("ContainsBase", [](ReflMngr& mngr, Type type, Type base) {return mngr.ContainsBase(type, base); });
	Mngr.AddMethod<&ReflMngr::ContainsField>("ContainsField");
	Mngr.AddMemberMethod("ContainsField", [](ReflMngr& mngr, Type type, Name name) {return mngr.ContainsField(type, name); });
	Mngr.AddMethod<&ReflMngr::ContainsMethod>("ContainsMethod");
	Mngr.AddMemberMethod("ContainsMethod", [](ReflMngr& mngr, Type type, Name name) {return mngr.ContainsMethod(type, name); });
}
