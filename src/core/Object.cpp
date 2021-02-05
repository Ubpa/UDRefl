#include <UDRefl/Object.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

//
// ObjectPtr
//////////////

TypeInfo* ObjectPtr::GetTypeInfo() const {
	auto target = Mngr->typeinfos.find(ID);
	if (target == Mngr->typeinfos.end())
		return nullptr;

	return &target->second;
}

std::string_view ObjectPtr::TypeName() const {
	return Mngr->tregistry.Nameof(ID);
}

ObjectPtr ObjectPtr::Var(StrID fieldID) const {
	return Mngr->Var(*this, fieldID);
}

ObjectPtr ObjectPtr::Var(TypeID baseID, StrID fieldID) const {
	return Mngr->Var(*this, baseID, fieldID);
}

InvokeResult ObjectPtr::Invoke(
	StrID methodID,
	void* result_buffer,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer) const
{
	return Mngr->Invoke(*this, methodID, result_buffer, argTypeIDs, argptr_buffer);
}

SharedObject ObjectPtr::MInvoke(
	StrID methodID,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer,
	std::pmr::memory_resource* rst_rsrc) const
{
	return Mngr->MInvoke(*this, methodID, argTypeIDs, argptr_buffer, rst_rsrc);
}

void ObjectPtr::ForEachVar(const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const {
	return Mngr->ForEachVar({ID, ptr}, func);
}

void ObjectPtr::ForEachOwnedVar(const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const {
	return Mngr->ForEachOwnedVar(*this, func);
}

std::vector<TypeID> ObjectPtr::GetTypeIDs() {
	return Mngr->GetTypeIDs(ID);
}

std::vector<TypeRef> ObjectPtr::GetTypes() {
	return Mngr->GetTypes(ID);
}

std::vector<TypeFieldRef> ObjectPtr::GetTypeFields() {
	return Mngr->GetTypeFields(ID);
}

std::vector<FieldRef> ObjectPtr::GetFields() {
	return Mngr->GetFields(ID);
}

std::vector<TypeMethodRef> ObjectPtr::GetTypeMethods() {
	return Mngr->GetTypeMethods(ID);
}

std::vector<MethodRef> ObjectPtr::GetMethods() {
	return Mngr->GetMethods(ID);
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> ObjectPtr::GetTypeFieldVars() {
	return Mngr->GetTypeFieldVars(*this);
}

std::vector<ObjectPtr> ObjectPtr::GetVars() {
	return Mngr->GetVars(*this);
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> ObjectPtr::GetTypeFieldOwnedVars() {
	return Mngr->GetTypeFieldOwnedVars(*this);
}

std::vector<ObjectPtr> ObjectPtr::GetOwnedVars() {
	return Mngr->GetOwnedVars(*this);
}

std::optional<TypeID> ObjectPtr::FindTypeID(const std::function<bool(TypeID)>& func) const {
	return Mngr->FindTypeID(ID, func);
}

std::optional<TypeRef> ObjectPtr::FindType(const std::function<bool(TypeRef)>& func) const {
	return Mngr->FindType(ID, func);
}

std::optional<FieldRef> ObjectPtr::FindField(const std::function<bool(FieldRef)>& func) const {
	return Mngr->FindField(ID, func);
}

std::optional<MethodRef> ObjectPtr::FindMethod(const std::function<bool(MethodRef)>& func) const {
	return Mngr->FindMethod(ID, func);
}

ObjectPtr ObjectPtr::FindVar(const std::function<bool(ObjectPtr)>& func) const {
	return Mngr->FindVar(*this, func);
}

ObjectPtr ObjectPtr::FindOwnedVar(const std::function<bool(ObjectPtr)>& func) const {
	return Mngr->FindOwnedVar(*this, func);
}

bool ObjectPtr::ContainsBase(TypeID baseID) const {
	return Mngr->ContainsBase(ID, baseID);
}

bool ObjectPtr::ContainsField(StrID fieldID) const {
	return Mngr->ContainsField(ID, fieldID);
}

bool ObjectPtr::ContainsMethod(StrID methodID) const {
	return Mngr->ContainsMethod(ID, methodID);
}

bool ObjectPtr::ContainsVariableMethod(StrID methodID) const {
	return Mngr->ContainsVariableMethod(ID, methodID);
}

bool ObjectPtr::ContainsConstMethod(StrID methodID) const {
	return Mngr->ContainsConstMethod(ID, methodID);
}

bool ObjectPtr::ContainsStaticMethod(StrID methodID) const {
	return Mngr->ContainsStaticMethod(ID, methodID);
}

bool ObjectPtr::IsConst() const {
	return Mngr->IsConst(ID);
}

bool ObjectPtr::IsReadOnly() const {
	return Mngr->IsReadOnly(ID);
}

bool ObjectPtr::IsReference() const {
	return Mngr->IsReadOnly(ID);
}

ConstReferenceMode ObjectPtr::GetConstReferenceMode() const {
	return Mngr->GetConstReferenceMode(ID);
}

ObjectPtr ObjectPtr::RemoveConst() const {
	return { Mngr->RemoveConst(ID), ptr };
}

ObjectPtr ObjectPtr::RemoveReference() const {
	return { Mngr->RemoveReference(ID), ptr };
}

ObjectPtr ObjectPtr::RemoveConstReference() const {
	return { Mngr->RemoveConstReference(ID), ptr };
}

ObjectPtr ObjectPtr::AddConst() const {
	return { Mngr->AddConst(ID), ptr };
}

ObjectPtr ObjectPtr::AddConstLValueReference() const {
	return { Mngr->AddConstLValueReference(ID), ptr };
}

ObjectPtr ObjectPtr::AddConstRValueReference() const {
	return { Mngr->AddConstRValueReference(ID), ptr };
}

ObjectPtr ObjectPtr::AddLValueReference() const {
	return { Mngr->AddLValueReference(ID), ptr };
}

ObjectPtr ObjectPtr::AddLValueReferenceWeak() const {
	return { Mngr->AddLValueReferenceWeak(ID), ptr };
}

ObjectPtr ObjectPtr::AddRValueReference() const {
	return { Mngr->AddRValueReference(ID), ptr };
}

InvocableResult ObjectPtr::IsInvocable(StrID methodID, std::span<const TypeID> argTypeIDs) const {
	return Mngr->IsConstInvocable(ID, methodID, argTypeIDs);
}

ObjectPtr ObjectPtr::StaticCast_DerivedToBase(TypeID typeID) const {
	return Mngr->StaticCast_DerivedToBase(*this, typeID);
}

ObjectPtr ObjectPtr::StaticCast_BaseToDerived(TypeID typeID) const {
	return Mngr->StaticCast_BaseToDerived(*this, typeID);
}

ObjectPtr ObjectPtr::DynamicCast_BaseToDerived(TypeID typeID) const {
	return Mngr->DynamicCast_BaseToDerived(*this, typeID);
}

ObjectPtr ObjectPtr::StaticCast(TypeID typeID) const {
	return Mngr->StaticCast(*this, typeID);
}

ObjectPtr ObjectPtr::DynamicCast(TypeID typeID) const {
	return Mngr->DynamicCast(*this, typeID);
}

SharedObject ObjectPtr::operator+() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_plus);
}

SharedObject ObjectPtr::operator-() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_minus);
}

SharedObject ObjectPtr::operator~() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_bnot);
}

SharedObject ObjectPtr::operator[](std::size_t n) const {
	return DMInvoke(StrIDRegistry::MetaID::operator_subscript, std::move(n));
}

SharedObject ObjectPtr::operator*() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_deref);
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

ObjectPtr ObjectPtr::tuple_get(std::size_t i) const {
	return Mngr->Invoke<ObjectPtr>(*this, StrIDRegistry::MetaID::tuple_get, std::move(i));
}

SharedObject ObjectPtr::begin() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_begin);
}

SharedObject ObjectPtr::end() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_end);
}

SharedObject ObjectPtr::rbegin() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_rbegin);
}

SharedObject ObjectPtr::rend() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_rend);
}

SharedObject ObjectPtr::data() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_data);
}

SharedObject ObjectPtr::front() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_front);
}

SharedObject ObjectPtr::back() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_back);
}

std::size_t ObjectPtr::tuple_size() const {
	return Mngr->Invoke<std::size_t>(*this, StrIDRegistry::MetaID::tuple_size);
}

SharedObject ObjectPtr::cbegin() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_cbegin);
}

SharedObject ObjectPtr::cend() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_cend);
}

SharedObject ObjectPtr::crbegin() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_crbegin);
}

SharedObject ObjectPtr::crend() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_crend);
}

// - element access

SharedObject ObjectPtr::empty() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_empty);
}

SharedObject ObjectPtr::size() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_size);
}

SharedObject ObjectPtr::capacity() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_capacity);
}

SharedObject ObjectPtr::bucket_count() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_bucket_count);
}

// - observers

SharedObject ObjectPtr::key_comp() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_key_comp);
}

SharedObject ObjectPtr::value_comp() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_value_comp);
}

SharedObject ObjectPtr::hash_function() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_hash_function);
}

SharedObject ObjectPtr::key_eq() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_key_eq);
}

SharedObject ObjectPtr::get_allocator() const {
	return Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_get_allocator);
}

void ObjectPtr::reserve(std::size_t n) const {
	Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_reserve, std::move(n));
}

void ObjectPtr::shrink_to_fit() const {
	Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_shrink_to_fit);
}

void ObjectPtr::clear() const {
	Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_clear);
}

void ObjectPtr::pop_front() const {
	Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_pop_front);
}

void ObjectPtr::pop_back() const {
	Mngr->DMInvoke(*this, StrIDRegistry::MetaID::container_pop_back);
}

//
// SharedObject
/////////////////

SharedObject SharedObject::begin() const {
	return AsObjectPtr().begin();
}

SharedObject SharedObject::end() const {
	return AsObjectPtr().end();
}
