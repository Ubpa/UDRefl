#include <UDRefl/Object.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

ObjectView ObjectView::Var(Name field_name, FieldFlag flag) const {
	return Mngr->Var(*this, field_name, flag);
}

ObjectView ObjectView::Var(Type base, Name field_name, FieldFlag flag) const {
	return Mngr->Var(*this, base, field_name, flag);
}

Type ObjectView::BInvoke(
	Name method_name,
	void* result_buffer,
	std::span<const Type> argTypes,
	ArgPtrBuffer argptr_buffer,
	MethodFlag flag) const
{
	return Mngr->BInvoke(*this, method_name, result_buffer, argTypes, argptr_buffer, flag);
}

SharedObject ObjectView::MInvoke(
	Name method_name,
	std::pmr::memory_resource* rst_rsrc,
	std::pmr::memory_resource* temp_args_rsrc,
	std::span<const Type> argTypes,
	ArgPtrBuffer argptr_buffer,
	MethodFlag flag) const
{
	return Mngr->MInvoke(*this, method_name, rst_rsrc, temp_args_rsrc, argTypes, argptr_buffer, flag);
}

SharedObject ObjectView::Invoke(
	Name method_name,
	std::span<const Type> argTypes,
	ArgPtrBuffer argptr_buffer,
	MethodFlag flag) const
{
	return Mngr->Invoke(*this, method_name, argTypes, argptr_buffer, flag);
}

void ObjectView::ForEachVar(const std::function<bool(InfoTypePair, InfoFieldPair, ObjectView)>& func, FieldFlag flag) const {
	return Mngr->ForEachVar(*this, func, flag);
}

std::vector<std::tuple<InfoTypePair, InfoFieldPair, ObjectView>> ObjectView::GetTypeFieldVars(FieldFlag flag) const {
	return Mngr->GetTypeFieldVars(*this, flag);
}

std::vector<ObjectView> ObjectView::GetVars(FieldFlag flag) const {
	return Mngr->GetVars(*this, flag);
}

ObjectView ObjectView::FindVar(const std::function<bool(ObjectView)>& func, FieldFlag flag) const {
	return Mngr->FindVar(*this, func, flag);
}

ObjectView ObjectView::RemoveConst() const {
	return { type.RemoveConst(), ptr };
}

ObjectView ObjectView::RemoveLValueReference() const {
	return { type.RemoveLValueReference(), ptr };
}

ObjectView ObjectView::RemoveRValueReference() const {
	return { type.RemoveRValueReference(), ptr };
}

ObjectView ObjectView::RemoveReference() const {
	return { type.RemoveReference(), ptr };
}

ObjectView ObjectView::RemoveConstReference() const {
	return { type.RemoveCVRef(), ptr };
}

ObjectView ObjectView::AddConst() const {
	return { Mngr->tregistry.RegisterAddConst(type), ptr };
}

ObjectView ObjectView::AddConstLValueReference() const {
	return { Mngr->tregistry.RegisterAddConstLValueReference(type), ptr };
}

ObjectView ObjectView::AddConstRValueReference() const {
	return { Mngr->tregistry.RegisterAddConstRValueReference(type), ptr };
}

ObjectView ObjectView::AddLValueReference() const {
	return { Mngr->tregistry.RegisterAddLValueReference(type), ptr };
}

ObjectView ObjectView::AddLValueReferenceWeak() const {
	return { Mngr->tregistry.RegisterAddLValueReferenceWeak(type), ptr };
}

ObjectView ObjectView::AddRValueReference() const {
	return { Mngr->tregistry.RegisterAddRValueReference(type), ptr };
}

Type ObjectView::IsInvocable(Name method_name, std::span<const Type> argTypes, MethodFlag flag) const {
	return Mngr->IsInvocable(type, method_name, argTypes, flag);
}

ObjectView ObjectView::StaticCast_DerivedToBase(Type type) const {
	return Mngr->StaticCast_DerivedToBase(*this, type);
}

ObjectView ObjectView::StaticCast_BaseToDerived(Type type) const {
	return Mngr->StaticCast_BaseToDerived(*this, type);
}

ObjectView ObjectView::DynamicCast_BaseToDerived(Type type) const {
	return Mngr->DynamicCast_BaseToDerived(*this, type);
}

ObjectView ObjectView::StaticCast(Type type) const {
	return Mngr->StaticCast(*this, type);
}

ObjectView ObjectView::DynamicCast(Type type) const {
	return Mngr->DynamicCast(*this, type);
}

SharedObject ObjectView::operator+() const {
	return Invoke(NameIDRegistry::Meta::operator_plus);
}

SharedObject ObjectView::operator-() const {
	return Invoke(NameIDRegistry::Meta::operator_minus);
}

SharedObject ObjectView::operator~() const {
	return Invoke(NameIDRegistry::Meta::operator_bnot);
}

SharedObject ObjectView::operator[](std::size_t n) const {
	return Invoke(NameIDRegistry::Meta::operator_subscript, std::move(n));
}

SharedObject ObjectView::operator*() const {
	return Invoke(NameIDRegistry::Meta::operator_deref);
}

SharedObject ObjectView::operator++() const {
	return Invoke(NameIDRegistry::Meta::operator_pre_inc);
}

SharedObject ObjectView::operator++(int) const {
	return Invoke<int>(NameIDRegistry::Meta::operator_post_inc, 0);
}

SharedObject ObjectView::operator--() const {
	return Invoke(NameIDRegistry::Meta::operator_pre_dec);
}

SharedObject ObjectView::operator--(int) const {
	return Invoke<int>(NameIDRegistry::Meta::operator_post_dec, 0);
}

ObjectView ObjectView::tuple_get(std::size_t i) const {
	return Mngr->BInvoke<ObjectView>(*this, NameIDRegistry::Meta::tuple_get, std::move(i));
}

SharedObject ObjectView::begin() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::container_begin);
}

SharedObject ObjectView::end() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::container_end);
}

SharedObject ObjectView::rbegin() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::container_rbegin);
}

SharedObject ObjectView::rend() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::container_rend);
}

SharedObject ObjectView::data() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::container_data);
}

SharedObject ObjectView::front() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::container_front);
}

SharedObject ObjectView::back() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::container_back);
}

std::size_t ObjectView::tuple_size() const {
	return Mngr->BInvoke<std::size_t>(*this, NameIDRegistry::Meta::tuple_size);
}

SharedObject ObjectView::next() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::iterator_next);
}

SharedObject ObjectView::prev() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::iterator_prev);
}

SharedObject ObjectView::cbegin() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::container_cbegin);
}

SharedObject ObjectView::cend() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::container_cend);
}

SharedObject ObjectView::crbegin() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::container_crbegin);
}

SharedObject ObjectView::crend() const {
	return Mngr->Invoke(*this, NameIDRegistry::Meta::container_crend);
}

// - element access

bool ObjectView::empty() const {
	return Mngr->BInvoke<bool>(*this, NameIDRegistry::Meta::container_empty);
}

std::size_t ObjectView::size() const {
	return Mngr->BInvoke<std::size_t>(*this, NameIDRegistry::Meta::container_size);
}

std::size_t ObjectView::capacity() const {
	return Mngr->BInvoke<std::size_t>(*this, NameIDRegistry::Meta::container_capacity);
}

std::size_t ObjectView::bucket_count() const {
	return Mngr->BInvoke<std::size_t>(*this, NameIDRegistry::Meta::container_bucket_count);
}

void ObjectView::reserve(std::size_t n) const {
	Mngr->Invoke(*this, NameIDRegistry::Meta::container_reserve, std::move(n));
}

void ObjectView::shrink_to_fit() const {
	Mngr->Invoke(*this, NameIDRegistry::Meta::container_shrink_to_fit);
}

void ObjectView::clear() const {
	Mngr->Invoke(*this, NameIDRegistry::Meta::container_clear);
}

void ObjectView::pop_front() const {
	Mngr->Invoke(*this, NameIDRegistry::Meta::container_pop_front);
}

void ObjectView::pop_back() const {
	Mngr->Invoke(*this, NameIDRegistry::Meta::container_pop_back);
}
