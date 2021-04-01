#include <UDRefl/Object.hpp>

#include <UDRefl/ReflMngr.hpp>

#include <UDRefl/ranges/ObjectTree.hpp>
#include <UDRefl/ranges/FieldRange.hpp>
#include <UDRefl/ranges/MethodRange.hpp>
#include <UDRefl/ranges/VarRange.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

UDRefl_core_API std::pmr::memory_resource* Ubpa::UDRefl::ReflMngr_GetTemporaryResource() {
	return Mngr.GetTemporaryResource();
}

ObjectView::operator bool() const noexcept {
	if (ptr && type) {
		if (type.Is<bool>())
			return As<bool>();
		else {
			if (auto rst = IsInvocable(NameIDRegistry::Meta::operator_bool, MethodFlag::Const)) {
				assert(rst.Is<bool>());
				return Invoke<bool>(NameIDRegistry::Meta::operator_bool, ArgsView{}, MethodFlag::Const);
			}
			else
				return true;
		}
	}
	else
		return false;
}

ObjectView ObjectView::Var(Name field_name, FieldFlag flag) const {
	return Mngr.Var(*this, field_name, flag);
}

ObjectView ObjectView::Var(Type base, Name field_name, FieldFlag flag) const {
	return Mngr.Var(*this, base, field_name, flag);
}

Type ObjectView::BInvoke(
	Name method_name,
	void* result_buffer,
	ArgsView args,
	MethodFlag flag,
	std::pmr::memory_resource* temp_args_rsrc) const
{
	return Mngr.BInvoke(*this, method_name, result_buffer, args, flag, temp_args_rsrc);
}

SharedObject ObjectView::MInvoke(
	Name method_name,
	std::pmr::memory_resource* rst_rsrc,
	ArgsView args,
	MethodFlag flag,
	std::pmr::memory_resource* temp_args_rsrc) const
{
	return Mngr.MInvoke(*this, method_name, rst_rsrc, args, flag, temp_args_rsrc);
}

SharedObject ObjectView::Invoke(
	Name method_name,
	ArgsView args,
	MethodFlag flag,
	std::pmr::memory_resource* temp_args_rsrc) const
{
	return Mngr.Invoke(*this, method_name, args, flag, temp_args_rsrc);
}

ObjectView ObjectView::AddConst() const {
	return { Mngr.tregistry.RegisterAddConst(type), ptr };
}

ObjectView ObjectView::AddConstLValueReference() const {
	return { Mngr.tregistry.RegisterAddConstLValueReference(type), ptr };
}

ObjectView ObjectView::AddConstRValueReference() const {
	return { Mngr.tregistry.RegisterAddConstRValueReference(type), ptr };
}

ObjectView ObjectView::AddLValueReference() const {
	return { Mngr.tregistry.RegisterAddLValueReference(type), ptr };
}

ObjectView ObjectView::AddLValueReferenceWeak() const {
	return { Mngr.tregistry.RegisterAddLValueReferenceWeak(type), ptr };
}

ObjectView ObjectView::AddRValueReference() const {
	return { Mngr.tregistry.RegisterAddRValueReference(type), ptr };
}

Type ObjectView::IsInvocable(Name method_name, std::span<const Type> argTypes, MethodFlag flag) const {
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

ObjectTree ObjectView::GetObjectTree() const {
	return ObjectTree{ *this };
}

MethodRange ObjectView::GetMethods(MethodFlag flag) const {
	return { *this, flag };
}

FieldRange ObjectView::GetFields(FieldFlag flag) const {
	return { *this, flag };
}

VarRange ObjectView::GetVars(FieldFlag flag) const {
	return { *this, flag };
}

ContainerType ObjectView::get_container_type() const {
	auto* typeinfo = Mngr.GetTypeInfo(type);
	if (!typeinfo)
		return ContainerType::None;

	auto target = typeinfo->attrs.find(Type_of<ContainerType>);
	if(target == typeinfo->attrs.end())
		return ContainerType::None;

	return target->As<ContainerType>();
}


SharedObject SharedObject::StaticCast_DerivedToBase(Type base) const {
	auto b = ObjectView::StaticCast_DerivedToBase(base);
	if (!b.GetType().Valid())
		return {};

	
	return { b.GetType(), SharedBuffer{buffer,b.GetPtr()} };
}

SharedObject SharedObject::StaticCast_BaseToDerived(Type derived) const {
	auto d = ObjectView::StaticCast_BaseToDerived(derived);
	if (!d.GetType().Valid())
		return {};


	return { d.GetType(), SharedBuffer{buffer,d.GetPtr()} };
}

SharedObject SharedObject::DynamicCast_BaseToDerived(Type derived) const {
	auto d = ObjectView::StaticCast_BaseToDerived(derived);
	if (!d.GetType().Valid())
		return {};


	return { d.GetType(), SharedBuffer{buffer,d.GetPtr()} };
}

SharedObject SharedObject::StaticCast(Type type) const {
	auto t = ObjectView::StaticCast(type);
	if (!t.GetType().Valid())
		return {};

	return { t.GetType(), SharedBuffer{buffer,t.GetPtr()} };
}

SharedObject SharedObject::DynamicCast(Type type) const {
	auto t = ObjectView::DynamicCast(type);
	if (!t.GetType().Valid())
		return {};

	return { t.GetType(), SharedBuffer{buffer,t.GetPtr()} };
}
