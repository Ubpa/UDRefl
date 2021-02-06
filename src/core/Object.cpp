#include <UDRefl/Object.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

TypeInfo* ObjectView::GetTypeInfo() const {
	return Mngr.GetTypeInfo(type);
}

ObjectView ObjectView::Var(Name field_name) const {
	return Mngr.Var(*this, field_name);
}

ObjectView ObjectView::Var(Type base, Name field_name) const {
	return Mngr.Var(*this, base, field_name);
}

InvokeResult ObjectView::Invoke(
	Name method_name,
	void* result_buffer,
	std::span<const Type> argTypes,
	ArgPtrBuffer argptr_buffer,
	FuncFlag flag) const
{
	return Mngr.Invoke(*this, method_name, result_buffer, argTypes, argptr_buffer, flag);
}

SharedObject ObjectView::MInvoke(
	Name method_name,
	std::pmr::memory_resource* rst_rsrc,
	std::span<const Type> argTypes,
	ArgPtrBuffer argptr_buffer,
	FuncFlag flag) const
{
	return Mngr.MInvoke(*this, method_name, rst_rsrc, argTypes, argptr_buffer, flag);
}

SharedObject ObjectView::DMInvoke(
	Name method_name,
	std::span<const Type> argTypes,
	ArgPtrBuffer argptr_buffer,
	FuncFlag flag) const
{
	return Mngr.DMInvoke(*this, method_name, argTypes, argptr_buffer, flag);
}

void ObjectView::ForEachVar(const std::function<bool(TypeRef, FieldRef, ObjectView)>& func) const {
	return Mngr.ForEachVar({type, ptr}, func);
}

void ObjectView::ForEachOwnedVar(const std::function<bool(TypeRef, FieldRef, ObjectView)>& func) const {
	return Mngr.ForEachOwnedVar(*this, func);
}

std::vector<TypeRef> ObjectView::GetTypes() const {
	return Mngr.GetTypes(type);
}

std::vector<TypeFieldRef> ObjectView::GetTypeFields() const {
	return Mngr.GetTypeFields(type);
}

std::vector<FieldRef> ObjectView::GetFields() const {
	return Mngr.GetFields(type);
}

std::vector<TypeMethodRef> ObjectView::GetTypeMethods() const {
	return Mngr.GetTypeMethods(type);
}

std::vector<MethodRef> ObjectView::GetMethods() const {
	return Mngr.GetMethods(type);
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

std::optional<TypeRef> ObjectView::FindType(const std::function<bool(TypeRef)>& func) const {
	return Mngr.FindType(type, func);
}

std::optional<FieldRef> ObjectView::FindField(const std::function<bool(FieldRef)>& func) const {
	return Mngr.FindField(type, func);
}

std::optional<MethodRef> ObjectView::FindMethod(const std::function<bool(MethodRef)>& func) const {
	return Mngr.FindMethod(type, func);
}

ObjectView ObjectView::FindVar(const std::function<bool(ObjectView)>& func) const {
	return Mngr.FindVar(*this, func);
}

ObjectView ObjectView::FindOwnedVar(const std::function<bool(ObjectView)>& func) const {
	return Mngr.FindOwnedVar(*this, func);
}

bool ObjectView::ContainsBase(Type base) const {
	return Mngr.ContainsBase(type, base);
}

bool ObjectView::ContainsField(Name field_name) const {
	return Mngr.ContainsField(type, field_name);
}

bool ObjectView::ContainsMethod(Name method_name) const {
	return Mngr.ContainsMethod(type, method_name);
}

bool ObjectView::ContainsMethod(Name method_name, FuncFlag mode) const {
	return Mngr.ContainsMethod(type, method_name, mode);
}

ObjectView ObjectView::RemoveConst() const {
	return { type.RemoveConst(), ptr };
}

ObjectView ObjectView::RemoveReference() const {
	return { type.RemoveReference(), ptr };
}

ObjectView ObjectView::RemoveConstReference() const {
	return { type.RemoveCVRef(), ptr };
}

ObjectView ObjectView::AddConst() const {
	return { Mngr.AddConst(type), ptr };
}

ObjectView ObjectView::AddConstLValueReference() const {
	return { Mngr.AddConstLValueReference(type), ptr };
}

ObjectView ObjectView::AddConstRValueReference() const {
	return { Mngr.AddConstRValueReference(type), ptr };
}

ObjectView ObjectView::AddLValueReference() const {
	return { Mngr.AddLValueReference(type), ptr };
}

ObjectView ObjectView::AddLValueReferenceWeak() const {
	return { Mngr.AddLValueReferenceWeak(type), ptr };
}

ObjectView ObjectView::AddRValueReference() const {
	return { Mngr.AddRValueReference(type), ptr };
}

InvocableResult ObjectView::IsInvocable(Name method_name, std::span<const Type> argTypes, FuncFlag flag) const {
	return Mngr.IsInvocable(type, method_name, argTypes, flag);
}

ObjectView ObjectView::StaticCast_DerivedToBase(Type type) const {
	return Mngr.StaticCast_DerivedToBase(*this, type);
}

ObjectView ObjectView::StaticCast_BaseToDerived(Type type) const {
	return Mngr.StaticCast_BaseToDerived(*this, type);
}

ObjectView ObjectView::DynamicCast_BaseToDerived(Type type) const {
	return Mngr.DynamicCast_BaseToDerived(*this, type);
}

ObjectView ObjectView::StaticCast(Type type) const {
	return Mngr.StaticCast(*this, type);
}

ObjectView ObjectView::DynamicCast(Type type) const {
	return Mngr.DynamicCast(*this, type);
}

SharedObject ObjectView::operator+() const {
	return DMInvoke(NameIDRegistry::Meta::operator_plus);
}

SharedObject ObjectView::operator-() const {
	return DMInvoke(NameIDRegistry::Meta::operator_minus);
}

SharedObject ObjectView::operator~() const {
	return DMInvoke(NameIDRegistry::Meta::operator_bnot);
}

SharedObject ObjectView::operator[](std::size_t n) const {
	return DMInvoke(NameIDRegistry::Meta::operator_subscript, std::move(n));
}

SharedObject ObjectView::operator*() const {
	return DMInvoke(NameIDRegistry::Meta::operator_deref);
}

SharedObject ObjectView::operator++() const {
	return DMInvoke(NameIDRegistry::Meta::operator_pre_inc);
}

SharedObject ObjectView::operator++(int) const {
	return DMInvoke<int>(NameIDRegistry::Meta::operator_post_inc, 0);
}

SharedObject ObjectView::operator--() const {
	return DMInvoke(NameIDRegistry::Meta::operator_pre_dec);
}

SharedObject ObjectView::operator--(int) const {
	return DMInvoke<int>(NameIDRegistry::Meta::operator_post_dec, 0);
}

ObjectView ObjectView::tuple_get(std::size_t i) const {
	return Mngr.Invoke<ObjectView>(*this, NameIDRegistry::Meta::tuple_get, std::move(i));
}

SharedObject ObjectView::begin() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_begin);
}

SharedObject ObjectView::end() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_end);
}

SharedObject ObjectView::rbegin() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_rbegin);
}

SharedObject ObjectView::rend() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_rend);
}

SharedObject ObjectView::data() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_data);
}

SharedObject ObjectView::front() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_front);
}

SharedObject ObjectView::back() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_back);
}

std::size_t ObjectView::tuple_size() const {
	return Mngr.Invoke<std::size_t>(*this, NameIDRegistry::Meta::tuple_size);
}

SharedObject ObjectView::cbegin() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_cbegin);
}

SharedObject ObjectView::cend() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_cend);
}

SharedObject ObjectView::crbegin() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_crbegin);
}

SharedObject ObjectView::crend() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_crend);
}

// - element access

SharedObject ObjectView::empty() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_empty);
}

SharedObject ObjectView::size() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_size);
}

SharedObject ObjectView::capacity() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_capacity);
}

SharedObject ObjectView::bucket_count() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_bucket_count);
}

// - observers

SharedObject ObjectView::key_comp() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_key_comp);
}

SharedObject ObjectView::value_comp() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_value_comp);
}

SharedObject ObjectView::hash_function() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_hash_function);
}

SharedObject ObjectView::key_eq() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_key_eq);
}

SharedObject ObjectView::get_allocator() const {
	return Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_get_allocator);
}

void ObjectView::reserve(std::size_t n) const {
	Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_reserve, std::move(n));
}

void ObjectView::shrink_to_fit() const {
	Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_shrink_to_fit);
}

void ObjectView::clear() const {
	Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_clear);
}

void ObjectView::pop_front() const {
	Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_pop_front);
}

void ObjectView::pop_back() const {
	Mngr.DMInvoke(*this, NameIDRegistry::Meta::container_pop_back);
}
