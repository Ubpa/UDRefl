#include <UDRefl/ReflMngr.h>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

template<typename T>
void RegisterIDRegistry() {
	using U = IDRegistry<T>;
	Mngr.RegisterType<U>();
	Mngr.AddMethod<MemFuncOf<U, void(T, std::string_view)>::get(&U::RegisterUnmanaged)>("RegisterUnmanaged");
	Mngr.AddMethod<MemFuncOf<U, T(std::string_view)>::get(&U::RegisterUnmanaged)>("RegisterUnmanaged");
	Mngr.AddMethod<MemFuncOf<U, void(T, std::string_view)>::get(&U::Register)>("Register");
	Mngr.AddMethod<MemFuncOf<U, T(std::string_view)>::get(&U::Register)>("Register");
	Mngr.AddMethod<&U::IsRegistered>("IsRegistered");
	Mngr.AddMethod<&U::Nameof>("Nameof");
	Mngr.AddMethod<&U::UnregisterUnmanaged>("UnregisterUnmanaged");
	Mngr.AddMethod<&U::Clear>("Clear");
}

ObjectView ReflMngr::ReflSefl() {
#ifdef UBPA_UDREFL_BOOTSTRAP
	// UTemplate
	RegisterType<NameID>();
	AddConstructor<NameID, std::size_t>();
	AddConstructor<NameID, std::string_view>();
	AddMethod<&NameID::GetValue>("GetValue");
	AddMethod<&NameID::Valid>("Valid");
	AddMethod<&NameID::Is>("Is");

	RegisterType<TypeID>();
	AddConstructor<TypeID, std::size_t>();
	AddConstructor<TypeID, std::string_view>();
	AddMethod<&TypeID::GetValue>("GetValue");
	AddMethod<&TypeID::Valid>("Valid");
	AddMethod<MemFuncOf<TypeID, bool(std::string_view)const noexcept>::get(&TypeID::Is)>("Is");

	RegisterType<Name>();
	AddConstructor<Name, std::string_view>();
	AddConstructor<Name, std::string_view, NameID>();
	AddMethod<&Name::GetView>("GetView");
	AddMethod<&Name::GetID>("GetID");
	AddMethod<&Name::Valid>("Valid");
	AddMethod<&Name::Is>("Is");

	RegisterType<Type>();
	AddConstructor<Type, std::string_view>();
	AddConstructor<Type, std::string_view, TypeID>();
	AddMethod<&Type::GetName>("GetName");
	AddMethod<&Type::GetID>("GetID");
	AddMethod<&Type::Valid>("Valid");
	AddMethod<MemFuncOf<Type, bool(std::string_view)const noexcept>::get(&Type::Is)>("Is");

	// Basic.h

	RegisterType<MethodFlag>();
	AddField<MethodFlag::Variable>("Variable");
	AddField<MethodFlag::Const>("Const");
	AddField<MethodFlag::Static>("Static");
	AddField<MethodFlag::None>("None");
	AddField<MethodFlag::All>("All");

	RegisterType<FieldFlag>();
	AddField<FieldFlag::Basic>("Basic");
	AddField<FieldFlag::Virtual>("Virtual");
	AddField<FieldFlag::Static>("Static");
	AddField<FieldFlag::DynamicShared>("DynamicShared");
	AddField<FieldFlag::DynamicBuffer>("DynamicBuffer");
	AddField<FieldFlag::None>("None");
	AddField<FieldFlag::Owned>("Owned");
	AddField<FieldFlag::Unowned>("Unowned");
	AddField<FieldFlag::All>("All");

	RegisterType<ResultDesc>();
	AddField<&ResultDesc::type>("type");
	AddField<&ResultDesc::size>("size");
	AddField<&ResultDesc::alignment>("alignment");

	RegisterType<InvokeResult>();
	AddField<&InvokeResult::success>("success");
	AddField<&InvokeResult::type>("type");
	AddField<&InvokeResult::destructor>("destructor");

	RegisterType<InvocableResult>();
	AddField<&InvocableResult::success>("success");
	AddField<&InvocableResult::result_desc>("result_desc");

	RegisterType<InfoTypePair>();
	AddField<&InfoTypePair::type>("type");
	AddField(Type_of<InfoTypePair>, "info", [](void* tref) {return &reinterpret_cast<InfoTypePair*>(tref)->info; });

	RegisterType<InfoFieldPair>();
	AddField<&InfoFieldPair::name>("name");
	AddField(Type_of<InfoFieldPair>, "info", [](void* tref) {return &reinterpret_cast<InfoFieldPair*>(tref)->info; });

	RegisterType<InfoMethodPair>();
	AddField<&InfoMethodPair::name>("name");
	AddField(Type_of<InfoMethodPair>, "info", [](void* tref) {return &reinterpret_cast<InfoMethodPair*>(tref)->info; });

	RegisterType<InfoTypeFieldPair>();
	AddField<&InfoTypeFieldPair::type>("type");
	AddField<&InfoTypeFieldPair::field>("field");

	RegisterType<InfoTypeMethodPair>();
	AddField<&InfoTypeMethodPair::type>("type");
	AddField<&InfoTypeMethodPair::method>("method");

	RegisterType(Type_of<ReflMngr>, 0, 0);
	AddMethod<MemFuncOf<ReflMngr, TypeInfo* (Type)>::get(&ReflMngr::GetTypeInfo)>("GetTypeInfo");
	AddMethod<MemFuncOf<ReflMngr, const TypeInfo* (Type) const>::get(&ReflMngr::GetTypeInfo)>("GetTypeInfo");

	// config.h

	AddField(GlobalType, "MaxArgNum", FieldInfo{ GenerateFieldPtr<&MaxArgNum>() });

	// Object.h

	RegisterType<ObjectView>();
	AddConstructor<ObjectView, Type, void*>();
	AddConstructor<ObjectView, Type>();
	AddConstructor<ObjectView, SharedObject>();
	AddMethod<&ObjectView::GetType>("GetType");
	AddMethod<&ObjectView::GetPtr>("GetPtr");

	RegisterType<SharedObject>();
	AddConstructor<SharedObject, Type, void*>();
	AddConstructor<SharedObject, Type>();
	AddConstructor<SharedObject, ObjectView>();
	AddMethod<&SharedObject::Reset>("Reset");
	AddMethod<&SharedObject::UseCount>("UseCount");
	AddMethod<&SharedObject::IsObjectView>("IsObjectView");
	AddMethod<&SharedObject::Swap>("Swap");

	// FieldPtr.h

	RegisterType<FieldPtr>();
	AddConstructor<FieldPtr, Type, std::size_t>();
	AddConstructor<FieldPtr, Type, void*>();
	AddConstructor<FieldPtr, ObjectView>();
	AddConstructor<FieldPtr, SharedObject>();
	AddMethod<&FieldPtr::GetType>("GetType");
	AddMethod<&FieldPtr::IsBasic>("IsBasic");
	AddMethod<&FieldPtr::IsVirtual>("IsVirtual");
	AddMethod<&FieldPtr::IsStatic>("IsStatic");
	AddMethod<&FieldPtr::IsDynamicShared>("IsDynamicShared");
	AddMethod<&FieldPtr::IsDyanmicBuffer>("IsDyanmicBuffer");
	AddMethod<&FieldPtr::IsOwned>("IsOwned");
	AddMethod<&FieldPtr::IsUnowned>("IsUnowned");
	AddMethod<&FieldPtr::GetFieldFlag>("GetFieldFlag");
	AddMethod<MemFuncOf<FieldPtr, ObjectView()noexcept>::get(&FieldPtr::Var)>("Var");
	AddMethod<MemFuncOf<FieldPtr, ObjectView(void*)>::get(&FieldPtr::Var)>("Var");

	// IDRegistry.h

	RegisterIDRegistry<NameID>();
	RegisterType<NameIDRegistry>();
	AddBases<NameIDRegistry, IDRegistry<NameID>>();

	RegisterIDRegistry<TypeID>();
	RegisterType<TypeIDRegistry>();
	AddBases<TypeIDRegistry, IDRegistry<TypeID>>();
	Mngr.AddMethod<MemFuncOf<TypeIDRegistry, void(TypeID, std::string_view)>::get(&TypeIDRegistry::RegisterUnmanaged)>("RegisterUnmanaged");
	Mngr.AddMethod<MemFuncOf<TypeIDRegistry, TypeID(std::string_view)>::get(&TypeIDRegistry::RegisterUnmanaged)>("RegisterUnmanaged");
	Mngr.AddMethod<MemFuncOf<TypeIDRegistry, void(TypeID, std::string_view)>::get(&TypeIDRegistry::Register)>("Register");
	Mngr.AddMethod<MemFuncOf<TypeIDRegistry, TypeID(std::string_view)>::get(&TypeIDRegistry::Register)>("Register");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddConst>("RegisterAddConst");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddLValueReference>("RegisterAddLValueReference");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddLValueReferenceWeak>("RegisterAddLValueReferenceWeak");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddRValueReference>("RegisterAddRValueReference");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddConstLValueReference>("RegisterAddConstLValueReference");
	Mngr.AddMethod<&TypeIDRegistry::RegisterAddConstRValueReference>("RegisterAddConstRValueReference");

	// MethodPtr.h

	RegisterType<ParamList>();

	RegisterType<ArgPtrBuffer>();

	RegisterType<ArgsView>();
	AddConstructor<ArgsView, ArgPtrBuffer, const ParamList&>();
	AddMethod<&ArgsView::GetBuffer>("GetBuffer");
	AddMethod<&ArgsView::GetParamList>("GetParamList");
	AddMethod<&ArgsView::At>("At");

	RegisterType<MethodPtr>();
	AddMethod<&MethodPtr::IsMemberVariable>("IsMemberVariable");
	AddMethod<&MethodPtr::IsMemberConst>("IsMemberConst");
	AddMethod<&MethodPtr::IsStatic>("IsStatic");
	AddMethod<&MethodPtr::GetMethodFlag>("GetMethodFlag");
	AddMethod<&MethodPtr::GetParamList>("GetParamList");
	AddMethod<&MethodPtr::GetResultDesc>("GetResultDesc");
	AddMethod<&MethodPtr::IsDistinguishableWith>("IsDistinguishableWith");

	// Info.h

	RegisterType<AttrSet>();

	RegisterType<BaseInfo>();
	AddMethod<&BaseInfo::IsVirtual>("IsVirtual");
	AddMethod<&BaseInfo::IsPolymorphic>("IsPolymorphic");
	AddMethod<&BaseInfo::StaticCast_DerivedToBase>("StaticCast_DerivedToBase");
	AddMethod<&BaseInfo::StaticCast_BaseToDerived>("StaticCast_BaseToDerived");
	AddMethod<&BaseInfo::DynamicCast_BaseToDerived>("DynamicCast_BaseToDerived");

	RegisterType<FieldInfo>();
	AddField<&FieldInfo::fieldptr>("fieldptr");
	AddField<&FieldInfo::attrs>("attrs");

	RegisterType<MethodInfo>();
	AddField<&MethodInfo::methodptr>("methodptr");
	AddField<&MethodInfo::attrs>("attrs");

	RegisterType<TypeInfo>();
	AddField<&TypeInfo::size>("size");
	AddField<&TypeInfo::alignment>("alignment");
	AddField<&TypeInfo::size>("size");
	AddField<&TypeInfo::fieldinfos>("fieldinfos");
	AddField<&TypeInfo::methodinfos>("methodinfos");
	AddField<&TypeInfo::baseinfos>("baseinfos");
	AddField<&TypeInfo::attrs>("attrs");

	// ReflMngr.h

	RegisterType<ReflMngr>();
	AddField<&ReflMngr::nregistry>("nregistry");
	AddField<&ReflMngr::tregistry>("tregistry");
	AddField<&ReflMngr::typeinfos>("typeinfos");
	AddStaticMethod(Type_of<ReflMngr>, "Instance", &ReflMngr::Instance);
	AddMethod<MemFuncOf<ReflMngr, TypeInfo* (Type)>::get(&ReflMngr::GetTypeInfo)>("GetTypeInfo");
	AddMethod<MemFuncOf<ReflMngr, const TypeInfo* (Type) const>::get(&ReflMngr::GetTypeInfo)>("GetTypeInfo");
	AddMethod<&ReflMngr::Clear>("Clear");
	AddMethod<MemFuncOf<ReflMngr, bool(Type, std::size_t, std::size_t)>::get(&ReflMngr::RegisterType)>("RegisterType");
	AddMethod<MemFuncOf<ReflMngr, bool(Type, Name, FieldInfo)>::get(&ReflMngr::AddField)>("AddField");
	AddMethod<MemFuncOf<ReflMngr, bool(Type, Name, MethodInfo)>::get(&ReflMngr::AddMethod)>("AddField");
	AddMethod<MemFuncOf<ReflMngr, bool(Type, Type, BaseInfo)>::get(&ReflMngr::AddBase)>("AddField");
	AddMethod<&ReflMngr::AddAttr>("AddAttr");
	AddMethod<&ReflMngr::StaticCast_DerivedToBase>("StaticCast_DerivedToBase");
	AddMethod<&ReflMngr::StaticCast_BaseToDerived>("StaticCast_BaseToDerived");
	AddMethod<&ReflMngr::DynamicCast_BaseToDerived>("DynamicCast_BaseToDerived");
	AddMethod<&ReflMngr::StaticCast>("StaticCast");
	AddMethod<&ReflMngr::DynamicCast>("DynamicCast");
	AddMethod<MemFuncOf<ReflMngr, ObjectView(ObjectView, Name, FieldFlag)>::get(&ReflMngr::Var)>("Var");
	AddMethod<MemFuncOf<ReflMngr, ObjectView(ObjectView, Type, Name, FieldFlag)>::get(&ReflMngr::Var)>("Var");
	AddMethod<MemFuncOf<ReflMngr, ObjectView(ObjectView, Type, Name, FieldFlag)>::get(&ReflMngr::Var)>("Var");

	RegisterType<std::vector<InfoTypePair>>();
	RegisterType<std::vector<InfoTypeFieldPair>>();
	RegisterType<std::vector<InfoFieldPair>>();
	RegisterType<std::vector<InfoTypeMethodPair>>();
	RegisterType<std::vector<InfoMethodPair>>();
	RegisterType<std::vector<std::tuple<InfoTypePair, InfoFieldPair, ObjectView>>>();
	RegisterType<std::vector<ObjectView>>();

	AddMethod<&ReflMngr::GetTypes>("GetTypes");
	AddMethod<&ReflMngr::GetTypeFields>("GetTypeFields");
	AddMethod<&ReflMngr::GetFields>("GetFields");
	AddMethod<&ReflMngr::GetTypeMethods>("GetTypeMethods");
	AddMethod<&ReflMngr::GetMethods>("GetMethods");
	AddMethod<&ReflMngr::GetTypeFieldVars>("GetTypeFieldVars");
	AddMethod<&ReflMngr::GetVars>("GetVars");

	AddMethod<&ReflMngr::ContainsBase>("ContainsBase");
	AddMethod<&ReflMngr::ContainsField>("ContainsField");
	AddMethod<&ReflMngr::ContainsMethod>("ContainsMethod");

	return ObjectView{ Type_of<ReflMngr>, &Mngr };

#else
	assert("You need to define UBPA_UDREFL_BOOTSTRAP for ReflSefl"  && false);
	return {};
#endif // UBPA_UDREFL_BOOTSTRAP
}
