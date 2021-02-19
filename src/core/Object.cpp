#include <UDRefl/Object.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

ObjectView::operator bool() const noexcept {
	if (ptr && type) {
		if (type.Is<bool>())
			return As<bool>();
		else {
			if (auto rst = IsInvocable(NameIDRegistry::Meta::operator_bool)) {
				assert(rst.Is<bool>());
				return BInvoke<bool>(NameIDRegistry::Meta::operator_bool);
			}
			else
				return true;
		}
	}
	else
		return false;
}

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

ContainerType ObjectView::get_container_type() const {
	auto* typeinfo = Mngr->GetTypeInfo(type);
	if (!typeinfo)
		return ContainerType::None;

	auto target = typeinfo->attrs.find(Type_of<ContainerType>);
	if(target == typeinfo->attrs.end())
		return ContainerType::None;

	return target->As<ContainerType>();
}
