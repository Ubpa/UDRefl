#include <UDRefl/Object.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

//
// ObjectPtrBase
//////////////////

TypeInfo* ObjectPtrBase::GetType() const {
	auto target = ReflMngr::Instance().typeinfos.find(ID);
	if (target == ReflMngr::Instance().typeinfos.end())
		return nullptr;

	return &target->second;
}

std::string_view ObjectPtrBase::TypeName() const {
	return ReflMngr::Instance().tregistry.Nameof(ID);
}

ConstObjectPtr ObjectPtrBase::RVar(StrID fieldID) const {
	return ReflMngr::Instance().RVar({ ID, ptr }, fieldID);
}

ConstObjectPtr ObjectPtrBase::RVar(TypeID baseID, StrID fieldID) const {
	return ReflMngr::Instance().RVar({ ID, ptr }, baseID, fieldID);
}

// self [r] vars and all bases' [r] vars
void ObjectPtrBase::ForEachRVar(const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const {
	return ReflMngr::Instance().ForEachRVar({ID, ptr}, func);
}

std::vector<TypeID> ObjectPtrBase::GetTypeIDs() {
	return ReflMngr::Instance().GetTypeIDs(ID);
}

std::vector<TypeRef> ObjectPtrBase::GetTypes() {
	return ReflMngr::Instance().GetTypes(ID);
}

std::vector<TypeFieldRef> ObjectPtrBase::GetTypeFields() {
	return ReflMngr::Instance().GetTypeFields(ID);
}

std::vector<FieldRef> ObjectPtrBase::GetFields() {
	return ReflMngr::Instance().GetFields(ID);
}

std::vector<TypeMethodRef> ObjectPtrBase::GetTypeMethods() {
	return ReflMngr::Instance().GetTypeMethods(ID);
}

std::vector<MethodRef> ObjectPtrBase::GetMethods() {
	return ReflMngr::Instance().GetMethods(ID);
}

std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> ObjectPtrBase::GetTypeFieldRVars() {
	return ReflMngr::Instance().GetTypeFieldRVars({ ID, ptr });
}

std::vector<ConstObjectPtr> ObjectPtrBase::GetRVars() {
	return ReflMngr::Instance().GetRVars({ ID, ptr });
}

std::optional<TypeID> ObjectPtrBase::FindTypeID(const std::function<bool(TypeID)>& func) const {
	return ReflMngr::Instance().FindTypeID(ID, func);
}

std::optional<TypeRef> ObjectPtrBase::FindType(const std::function<bool(TypeRef)>& func) const {
	return ReflMngr::Instance().FindType(ID, func);
}

std::optional<FieldRef> ObjectPtrBase::FindField(const std::function<bool(FieldRef)>& func) const {
	return ReflMngr::Instance().FindField(ID, func);
}

std::optional<MethodRef> ObjectPtrBase::FindMethod(const std::function<bool(MethodRef)>& func) const {
	return ReflMngr::Instance().FindMethod(ID, func);
}

ConstObjectPtr ObjectPtrBase::FindRVar(const std::function<bool(ConstObjectPtr)>& func) const {
	return ReflMngr::Instance().FindRVar({ ID, ptr }, func);
}

DereferenceProperty ObjectPtrBase::GetDereferenceProperty() const {
	return ReflMngr::Instance().GetDereferenceProperty(ID);
}

TypeID ObjectPtrBase::DereferenceID() const {
	return ReflMngr::Instance().Dereference(ID);
}

ObjectPtr ObjectPtrBase::Dereference() const {
	return ReflMngr::Instance().Dereference({ ID, ptr });
}

ConstObjectPtr ObjectPtrBase::ObjectPtrBase::DereferenceAsConst() const {
	return ReflMngr::Instance().DereferenceAsConst({ ID, ptr });
}

TypeID ObjectPtrBase::AddConstLValueReferenceID() const {
	return ReflMngr::Instance().AddConstLValueReference(ID);
}

ConstObjectPtr ObjectPtrBase::AddConstLValueReference() const {
	return ReflMngr::Instance().AddConstLValueReference({ ID, ptr });
}

SharedObject ObjectPtrBase::cbegin() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_cbegin);
}

SharedObject ObjectPtrBase::cend() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_cend);
}

SharedObject ObjectPtrBase::crbegin() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_crbegin);
}

SharedObject ObjectPtrBase::crend() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_crend);
}

// - element access

SharedObject ObjectPtrBase::empty() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_empty);
}

SharedObject ObjectPtrBase::size() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_size);
}

SharedObject ObjectPtrBase::capacity() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_capacity);
}

SharedObject ObjectPtrBase::bucket_count() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_bucket_count);
}

// - observers

SharedObject ObjectPtrBase::key_comp() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_key_comp);
}

SharedObject ObjectPtrBase::value_comp() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_value_comp);
}

SharedObject ObjectPtrBase::hash_function() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_hash_function);
}

SharedObject ObjectPtrBase::key_eq() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_key_eq);
}

SharedObject ObjectPtrBase::get_allocator() const {
	return ReflMngr::Instance().DMInvoke(ConstObjectPtr{ ID, ptr }, StrIDRegistry::MetaID::container_get_allocator);
}

InvocableResult ObjectPtrBase::IsInvocable(StrID methodID, Span<const TypeID> argTypeIDs) const {
	return ReflMngr::Instance().IsConstInvocable(ID, methodID, argTypeIDs);
}

InvokeResult ObjectPtrBase::Invoke(
	StrID methodID,
	void* result_buffer,
	Span<const TypeID> argTypeIDs,
	void* args_buffer) const
{
	return ReflMngr::Instance().Invoke(ConstObjectPtr{ ID, ptr }, methodID, result_buffer, argTypeIDs, args_buffer);
}

SharedObject ObjectPtrBase::MInvoke(
	StrID methodID,
	Span<const TypeID> argTypeIDs,
	void* args_buffer,
	std::pmr::memory_resource* rst_rsrc) const
{
	return ReflMngr::Instance().MInvoke(ConstObjectPtr{ ID, ptr }, methodID, argTypeIDs, args_buffer, rst_rsrc);
}


SharedObject ObjectPtrBase::operator+() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_plus);
}

SharedObject ObjectPtrBase::operator-() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_minus);
}

//
// ConstObjectPtr
///////////////////

ConstObjectPtr::ConstObjectPtr(const SharedConstObject& obj) noexcept
	: ConstObjectPtr{ obj.GetID(), obj.GetPtr() } {}

ConstObjectPtr ConstObjectPtr::StaticCast_DerivedToBase(TypeID typeID) const {
	return ReflMngr::Instance().StaticCast_DerivedToBase(*this, typeID);
}

ConstObjectPtr ConstObjectPtr::StaticCast_BaseToDerived(TypeID typeID) const {
	return ReflMngr::Instance().StaticCast_BaseToDerived(*this, typeID);
}

ConstObjectPtr ConstObjectPtr::DynamicCast_BaseToDerived(TypeID typeID) const {
	return ReflMngr::Instance().DynamicCast_BaseToDerived(*this, typeID);
}

ConstObjectPtr ConstObjectPtr::StaticCast(TypeID typeID) const {
	return ReflMngr::Instance().StaticCast(*this, typeID);
}

ConstObjectPtr ConstObjectPtr::DynamicCast(TypeID typeID) const {
	return ReflMngr::Instance().DynamicCast(*this, typeID);
}

SharedObject ConstObjectPtr::operator~() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_bnot);
}

SharedObject ConstObjectPtr::operator[](std::size_t n) const {
	return DMInvoke<std::size_t>(StrIDRegistry::MetaID::operator_subscript, n);
}

SharedObject ConstObjectPtr::operator*() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_deref);
}

SharedObject ConstObjectPtr::begin() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_begin);
}

SharedObject ConstObjectPtr::end() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_end);
}

SharedObject ConstObjectPtr::rbegin() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_rbegin);
}

SharedObject ConstObjectPtr::rend() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_rend);
}

SharedObject ConstObjectPtr::data() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_data);
}

SharedObject ConstObjectPtr::front() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_front);
}

SharedObject ConstObjectPtr::back() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_back);
}

//
// ObjectPtr
//////////////

ObjectPtr ObjectPtr::StaticCast_DerivedToBase(TypeID typeID) const {
	return ReflMngr::Instance().StaticCast_DerivedToBase(*this, typeID);
}

ObjectPtr ObjectPtr::StaticCast_BaseToDerived(TypeID typeID) const {
	return ReflMngr::Instance().StaticCast_BaseToDerived(*this, typeID);
}

ObjectPtr ObjectPtr::DynamicCast_BaseToDerived(TypeID typeID) const {
	return ReflMngr::Instance().DynamicCast_BaseToDerived(*this, typeID);
}

ObjectPtr ObjectPtr::StaticCast(TypeID typeID) const {
	return ReflMngr::Instance().StaticCast(*this, typeID);
}

ObjectPtr ObjectPtr::DynamicCast(TypeID typeID) const {
	return ReflMngr::Instance().DynamicCast(*this, typeID);
}

ObjectPtr ObjectPtr::RWVar(StrID fieldID) const {
	return ReflMngr::Instance().RWVar(*this, fieldID);
}

ObjectPtr ObjectPtr::RWVar(TypeID baseID, StrID fieldID) const {
	return ReflMngr::Instance().RWVar(*this, baseID, fieldID);
}

InvocableResult ObjectPtr::IsInvocable(StrID methodID, Span<const TypeID> argTypeIDs) const {
	return ReflMngr::Instance().IsInvocable(ID, methodID, argTypeIDs);
}

InvokeResult ObjectPtr::Invoke(
	StrID methodID,
	void* result_buffer,
	Span<const TypeID> argTypeIDs,
	void* args_buffer) const
{
	return ReflMngr::Instance().Invoke(*this, methodID, result_buffer, argTypeIDs, args_buffer);
}

SharedObject ObjectPtr::MInvoke(
	StrID methodID,
	Span<const TypeID> argTypeIDs,
	void* args_buffer,
	std::pmr::memory_resource* rst_rsrc) const
{
	return ReflMngr::Instance().MInvoke(*this, methodID, argTypeIDs, args_buffer, rst_rsrc);
}

void ObjectPtr::ForEachRWVar(const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const {
	return ReflMngr::Instance().ForEachRWVar(*this, func);
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> ObjectPtr::GetTypeFieldRWVars() {
	return ReflMngr::Instance().GetTypeFieldRWVars(*this);
}

std::vector<ObjectPtr> ObjectPtr::GetRWVars() {
	return ReflMngr::Instance().GetRWVars(*this);
}

ObjectPtr ObjectPtr::FindRWVar(const std::function<bool(ObjectPtr)>& func) const {
	return ReflMngr::Instance().FindRWVar(*this, func);
}

SharedObject ObjectPtr::operator++() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_pre_inc);
}

SharedObject ObjectPtr::operator++(int) const {
	return DMInvoke<int>(StrIDRegistry::MetaID::operator_post_inc, 0);
}

SharedObject ObjectPtr::operator--() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_pre_dec);
}

SharedObject ObjectPtr::operator--(int) const {
	return DMInvoke<int>(StrIDRegistry::MetaID::operator_post_dec, 0);
}

SharedObject ObjectPtr::operator[](std::size_t n) const {
	return DMInvoke<std::size_t>(StrIDRegistry::MetaID::operator_subscript, n);
}

SharedObject ObjectPtr::operator*() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_deref);
}

// - iterator

SharedObject ObjectPtr::begin() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_begin);
}

SharedObject ObjectPtr::end() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_end);
}

SharedObject ObjectPtr::rbegin() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_rbegin);
}

SharedObject ObjectPtr::rend() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_rend);
}

// - element access

SharedObject ObjectPtr::data() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_data);
}

SharedObject ObjectPtr::front() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_front);
}

SharedObject ObjectPtr::back() const {
	return ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_back);
}

void ObjectPtr::reserve(std::size_t n) const {
	ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_reserve);
}

void ObjectPtr::shrink_to_fit() const {
	ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_shrink_to_fit);
}

// - modifiers


void ObjectPtr::clear() const {
	ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_clear);
}

void ObjectPtr::pop_front() const {
	ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_pop_front);
}

void ObjectPtr::pop_back() const {
	ReflMngr::Instance().DMInvoke(*this, StrIDRegistry::MetaID::container_pop_back);
}

//
// SharedObjectBase
/////////////////////

SharedObject SharedObjectBase::operator+() const {
	return +AsObjectPtr();
}

SharedObject SharedObjectBase::operator-() const {
	return -AsObjectPtr();
}

SharedObject SharedObjectBase::operator~() const {
	return ~AsObjectPtr();
}

SharedObject SharedObjectBase::begin() const {
	return AsObjectPtr().begin();
}

SharedObject SharedObjectBase::end() const {
	return AsObjectPtr().end();
}

//
// SharedConstObject
//////////////////////

SharedObject SharedConstObject::operator[](std::size_t n) const {
	return AsObjectPtr()[n];
}

SharedObject SharedConstObject::operator*() const {
	return *AsObjectPtr();
}
