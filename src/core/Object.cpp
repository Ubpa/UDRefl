#include <UDRefl/Object.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

TypeInfo* ObjectView::GetTypeInfo() const {
	return Mngr.GetTypeInfo(ID);
}

std::string_view ObjectView::TypeName() const {
	return Mngr.tregistry.Nameof(ID);
}

ObjectView ObjectView::Var(StrID fieldID) const {
	return Mngr.Var(*this, fieldID);
}

ObjectView ObjectView::Var(TypeID baseID, StrID fieldID) const {
	return Mngr.Var(*this, baseID, fieldID);
}

InvokeResult ObjectView::Invoke(
	StrID methodID,
	void* result_buffer,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer) const
{
	return Mngr.Invoke(*this, methodID, result_buffer, argTypeIDs, argptr_buffer);
}

SharedObject ObjectView::MInvoke(
	StrID methodID,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer,
	std::pmr::memory_resource* rst_rsrc) const
{
	return Mngr.MInvoke(*this, methodID, argTypeIDs, argptr_buffer, rst_rsrc);
}

void ObjectView::ForEachVar(const std::function<bool(TypeRef, FieldRef, ObjectView)>& func) const {
	return Mngr.ForEachVar({ID, ptr}, func);
}

void ObjectView::ForEachOwnedVar(const std::function<bool(TypeRef, FieldRef, ObjectView)>& func) const {
	return Mngr.ForEachOwnedVar(*this, func);
}

std::vector<TypeID> ObjectView::GetTypeIDs() const {
	return Mngr.GetTypeIDs(ID);
}

std::vector<TypeRef> ObjectView::GetTypes() const {
	return Mngr.GetTypes(ID);
}

std::vector<TypeFieldRef> ObjectView::GetTypeFields() const {
	return Mngr.GetTypeFields(ID);
}

std::vector<FieldRef> ObjectView::GetFields() const {
	return Mngr.GetFields(ID);
}

std::vector<TypeMethodRef> ObjectView::GetTypeMethods() const {
	return Mngr.GetTypeMethods(ID);
}

std::vector<MethodRef> ObjectView::GetMethods() const {
	return Mngr.GetMethods(ID);
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> ObjectView::GetTypeFieldVars() const {
	return Mngr.GetTypeFieldVars(*this);
}

std::vector<ObjectView> ObjectView::GetVars() const {
	return Mngr.GetVars(*this);
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> ObjectView::GetTypeFieldOwnedVars() const {
	return Mngr.GetTypeFieldOwnedVars(*this);
}

std::vector<ObjectView> ObjectView::GetOwnedVars() const {
	return Mngr.GetOwnedVars(*this);
}

std::optional<TypeID> ObjectView::FindTypeID(const std::function<bool(TypeID)>& func) const {
	return Mngr.FindTypeID(ID, func);
}

std::optional<TypeRef> ObjectView::FindType(const std::function<bool(TypeRef)>& func) const {
	return Mngr.FindType(ID, func);
}

std::optional<FieldRef> ObjectView::FindField(const std::function<bool(FieldRef)>& func) const {
	return Mngr.FindField(ID, func);
}

std::optional<MethodRef> ObjectView::FindMethod(const std::function<bool(MethodRef)>& func) const {
	return Mngr.FindMethod(ID, func);
}

ObjectView ObjectView::FindVar(const std::function<bool(ObjectView)>& func) const {
	return Mngr.FindVar(*this, func);
}

ObjectView ObjectView::FindOwnedVar(const std::function<bool(ObjectView)>& func) const {
	return Mngr.FindOwnedVar(*this, func);
}

bool ObjectView::ContainsBase(TypeID baseID) const {
	return Mngr.ContainsBase(ID, baseID);
}

bool ObjectView::ContainsField(StrID fieldID) const {
	return Mngr.ContainsField(ID, fieldID);
}

bool ObjectView::ContainsMethod(StrID methodID) const {
	return Mngr.ContainsMethod(ID, methodID);
}

bool ObjectView::ContainsVariableMethod(StrID methodID) const {
	return Mngr.ContainsVariableMethod(ID, methodID);
}

bool ObjectView::ContainsConstMethod(StrID methodID) const {
	return Mngr.ContainsConstMethod(ID, methodID);
}

bool ObjectView::ContainsStaticMethod(StrID methodID) const {
	return Mngr.ContainsStaticMethod(ID, methodID);
}

bool ObjectView::IsConst() const {
	return Mngr.IsConst(ID);
}

bool ObjectView::IsReadOnly() const {
	return Mngr.IsReadOnly(ID);
}

bool ObjectView::IsReference() const {
	return Mngr.IsReadOnly(ID);
}

ConstReferenceMode ObjectView::GetConstReferenceMode() const {
	return Mngr.GetConstReferenceMode(ID);
}

ObjectView ObjectView::RemoveConst() const {
	return { Mngr.RemoveConst(ID), ptr };
}

ObjectView ObjectView::RemoveReference() const {
	return { Mngr.RemoveReference(ID), ptr };
}

ObjectView ObjectView::RemoveConstReference() const {
	return { Mngr.RemoveConstReference(ID), ptr };
}

ObjectView ObjectView::AddConst() const {
	return { Mngr.AddConst(ID), ptr };
}

ObjectView ObjectView::AddConstLValueReference() const {
	return { Mngr.AddConstLValueReference(ID), ptr };
}

ObjectView ObjectView::AddConstRValueReference() const {
	return { Mngr.AddConstRValueReference(ID), ptr };
}

ObjectView ObjectView::AddLValueReference() const {
	return { Mngr.AddLValueReference(ID), ptr };
}

ObjectView ObjectView::AddLValueReferenceWeak() const {
	return { Mngr.AddLValueReferenceWeak(ID), ptr };
}

ObjectView ObjectView::AddRValueReference() const {
	return { Mngr.AddRValueReference(ID), ptr };
}

InvocableResult ObjectView::IsInvocable(StrID methodID, std::span<const TypeID> argTypeIDs) const {
	return Mngr.IsConstInvocable(ID, methodID, argTypeIDs);
}

ObjectView ObjectView::StaticCast_DerivedToBase(TypeID typeID) const {
	return Mngr.StaticCast_DerivedToBase(*this, typeID);
}

ObjectView ObjectView::StaticCast_BaseToDerived(TypeID typeID) const {
	return Mngr.StaticCast_BaseToDerived(*this, typeID);
}

ObjectView ObjectView::DynamicCast_BaseToDerived(TypeID typeID) const {
	return Mngr.DynamicCast_BaseToDerived(*this, typeID);
}

ObjectView ObjectView::StaticCast(TypeID typeID) const {
	return Mngr.StaticCast(*this, typeID);
}

ObjectView ObjectView::DynamicCast(TypeID typeID) const {
	return Mngr.DynamicCast(*this, typeID);
}

SharedObject ObjectView::operator+() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_plus);
}

SharedObject ObjectView::operator-() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_minus);
}

SharedObject ObjectView::operator~() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_bnot);
}

SharedObject ObjectView::operator[](std::size_t n) const {
	return DMInvoke(StrIDRegistry::MetaID::operator_subscript, std::move(n));
}

SharedObject ObjectView::operator*() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_deref);
}

SharedObject ObjectView::operator++() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_pre_inc);
}

SharedObject ObjectView::operator++(int) const {
	return DMInvoke<int>(StrIDRegistry::MetaID::operator_post_inc, 0);
}

SharedObject ObjectView::operator--() const {
	return DMInvoke(StrIDRegistry::MetaID::operator_pre_dec);
}

SharedObject ObjectView::operator--(int) const {
	return DMInvoke<int>(StrIDRegistry::MetaID::operator_post_dec, 0);
}

ObjectView ObjectView::tuple_get(std::size_t i) const {
	return Mngr.Invoke<ObjectView>(*this, StrIDRegistry::MetaID::tuple_get, std::move(i));
}

SharedObject ObjectView::begin() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_begin);
}

SharedObject ObjectView::end() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_end);
}

SharedObject ObjectView::rbegin() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_rbegin);
}

SharedObject ObjectView::rend() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_rend);
}

SharedObject ObjectView::data() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_data);
}

SharedObject ObjectView::front() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_front);
}

SharedObject ObjectView::back() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_back);
}

std::size_t ObjectView::tuple_size() const {
	return Mngr.Invoke<std::size_t>(*this, StrIDRegistry::MetaID::tuple_size);
}

SharedObject ObjectView::cbegin() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_cbegin);
}

SharedObject ObjectView::cend() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_cend);
}

SharedObject ObjectView::crbegin() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_crbegin);
}

SharedObject ObjectView::crend() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_crend);
}

// - element access

SharedObject ObjectView::empty() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_empty);
}

SharedObject ObjectView::size() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_size);
}

SharedObject ObjectView::capacity() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_capacity);
}

SharedObject ObjectView::bucket_count() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_bucket_count);
}

// - observers

SharedObject ObjectView::key_comp() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_key_comp);
}

SharedObject ObjectView::value_comp() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_value_comp);
}

SharedObject ObjectView::hash_function() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_hash_function);
}

SharedObject ObjectView::key_eq() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_key_eq);
}

SharedObject ObjectView::get_allocator() const {
	return Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_get_allocator);
}

void ObjectView::reserve(std::size_t n) const {
	Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_reserve, std::move(n));
}

void ObjectView::shrink_to_fit() const {
	Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_shrink_to_fit);
}

void ObjectView::clear() const {
	Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_clear);
}

void ObjectView::pop_front() const {
	Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_pop_front);
}

void ObjectView::pop_back() const {
	Mngr.DMInvoke(*this, StrIDRegistry::MetaID::container_pop_back);
}
