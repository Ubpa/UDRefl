#include <UDRefl/ReflMngr.hpp>

#include "InvokeUtil.hpp"

#include "ReflMngrInitUtil/ReflMngrInitUtil.hpp"

#include <string>

#include <USmallFlat/small_vector.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

namespace Ubpa::UDRefl::details {
	static ObjectView StaticCast_BaseToDerived(ObjectView obj, Type type) {
		assert(obj.GetType().GetCVRefMode() == CVRefMode::None);

		if (obj.GetType() == type)
			return obj;

		auto target = Mngr.typeinfos.find(type);
		if (target == Mngr.typeinfos.end())
			return {};

		const auto& typeinfo = target->second;

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			auto ptr = StaticCast_BaseToDerived(obj, base);
			if (ptr.GetType())
				return { base, baseinfo.IsVirtual() ? nullptr : baseinfo.StaticCast_BaseToDerived(obj.GetPtr()) };
		}

		return {};
	}

	static ObjectView DynamicCast_BaseToDerived(ObjectView obj, Type type) {
		assert(obj.GetType().GetCVRefMode() == CVRefMode::None);

		if (obj.GetType() == type)
			return obj;

		auto target = Mngr.typeinfos.find(obj.GetType());
		if (target == Mngr.typeinfos.end())
			return {};

		const auto& typeinfo = target->second;

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			auto ptr = DynamicCast_BaseToDerived(ObjectView{ base, baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) }, type);
			if (ptr.GetType())
				return { base, baseinfo.IsPolymorphic() ? baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) : nullptr };
		}

		return {};
	}
}

ReflMngr::ReflMngr() :
	temporary_resource{ std::make_shared<std::pmr::synchronized_pool_resource>() },
	object_resource{ std::make_shared<std::pmr::synchronized_pool_resource>() }
{
	RegisterType(GlobalType, 0, 1, false, true);

	details::ReflMngrInitUtil_0(*this);
	details::ReflMngrInitUtil_1(*this);
	details::ReflMngrInitUtil_2(*this);
	details::ReflMngrInitUtil_3(*this);
	details::ReflMngrInitUtil_4(*this);
	details::ReflMngrInitUtil_5(*this);
	details::ReflMngrInitUtil_6(*this);
	details::ReflMngrInitUtil_7(*this);
}

ReflMngr& ReflMngr::Instance() noexcept {
	static ReflMngr instance;
	return instance;
}

TypeInfo* ReflMngr::GetTypeInfo(Type type) const {
	auto target = typeinfos.find(type.RemoveCVRef());
	if (target == typeinfos.end())
		return nullptr;
	return const_cast<TypeInfo*>(&target->second);
}


SharedObject ReflMngr::GetTypeAttr(Type type, Type attr_type) const {
	TypeInfo* typeinfo = GetTypeInfo(type);
	if (!typeinfo)
		return {};
	
	auto target = typeinfo->attrs.find(attr_type);
	if (target == typeinfo->attrs.end())
		return {};

	return *target;
}

SharedObject ReflMngr::GetFieldAttr(Type type, Name field_name, Type attr_type) const {
	for (const auto& [typeinfo, baseobj] : ObjectTree{ type }) {
		if (!typeinfo)
			continue;

		auto ftarget = typeinfo->fieldinfos.find(field_name);
		if (ftarget == typeinfo->fieldinfos.end())
			continue;
		
		const auto& finfo = ftarget->second;

		auto target = finfo.attrs.find(attr_type);
		if (target == finfo.attrs.end())
			return {};

		return *target;
	}
	return {};
}

SharedObject ReflMngr::GetMethodAttr(Type type, Name method_name, Type attr_type) const {
	for (const auto& [typeinfo, baseobj] : ObjectTree{ type }) {
		if (!typeinfo)
			continue;

		auto mtarget = typeinfo->methodinfos.find(method_name);
		if (mtarget == typeinfo->methodinfos.end())
			continue;

		const auto& minfo = mtarget->second;

		auto target = minfo.attrs.find(attr_type);
		if (target == minfo.attrs.end())
			return {};

		return *target;
	}
	return {};
}

void ReflMngr::SetTemporaryResource(std::shared_ptr<std::pmr::memory_resource> rsrc) {
	assert(rsrc.get());
	temporary_resource = std::move(rsrc);
}

void ReflMngr::SetObjectResource(std::shared_ptr<std::pmr::memory_resource> rsrc) {
	assert(rsrc.get());
	object_resource = std::move(rsrc);
}

void ReflMngr::Clear() noexcept {
	// field attrs
	for (auto& [type, typeinfo] : typeinfos) {
		for (auto& [field, fieldinfo] : typeinfo.fieldinfos)
			fieldinfo.attrs.clear();
	}

	// type attrs
	for (auto& [ID, typeinfo] : typeinfos)
		typeinfo.attrs.clear();

	// type dynamic field
	for (auto& [type, typeinfo] : typeinfos) {
		auto iter = typeinfo.fieldinfos.begin();
		while (iter != typeinfo.fieldinfos.end()) {
			auto cur = iter;
			++iter;

			if (cur->second.fieldptr.GetFieldFlag() == FieldFlag::DynamicShared)
				typeinfo.fieldinfos.erase(cur);
		}
	}

	typeinfos.clear();
}

ReflMngr::~ReflMngr() {
	Clear();
}

bool ReflMngr::ContainsVirtualBase(Type type) const {
	auto* info = GetTypeInfo(type);
	if (!info)
		return false;

	for (const auto& [base, baseinfo] : info->baseinfos) {
		if (baseinfo.IsVirtual() || ContainsVirtualBase(base))
			return true;
	}
	
	return false;
}

Type ReflMngr::RegisterType(Type type, size_t size, size_t alignment, bool is_polymorphic, bool is_trivial) {
	assert(alignment > 0 && (alignment & (alignment - 1)) == 0);
	auto target = typeinfos.find(type.RemoveCVRef());
	if (target != typeinfos.end())
		return {};
	Type new_type = { tregistry.Register(type.GetID(), type.GetName()),type.GetID() };
	typeinfos.emplace_hint(target, new_type, TypeInfo{ size,alignment,is_polymorphic,is_trivial });
	if (is_trivial)
		AddTrivialCopyConstructor(type);
	return new_type;
}

Type ReflMngr::RegisterType(
	Type type,
	std::span<const Type> bases,
	std::span<const Type> field_types,
	std::span<const Name> field_names,
	bool is_trivial)
{
	assert(field_types.size() == field_names.size());

	if (typeinfos.contains(type))
		return {};

	std::size_t size = 0;
	std::size_t alignment = 1;

	const size_t num_field = field_types.size();

	std::pmr::vector<std::size_t> base_offsets(temporary_resource.get());
	base_offsets.resize(bases.size());

	for (size_t i = 0; i < bases.size(); i++) {
		const auto& basetype = bases[i];
		auto btarget = typeinfos.find(basetype);
		if (btarget == typeinfos.end())
			return {};
		const auto& baseinfo = btarget->second;
		if (baseinfo.is_polymorphic || ContainsVirtualBase(basetype))
			return {};
		if (!baseinfo.is_trivial)
			is_trivial = false;
		assert(baseinfo.alignment > 0 && (baseinfo.alignment & (baseinfo.alignment - 1)) == 0);
		size = (size + (baseinfo.alignment - 1)) & ~(baseinfo.alignment - 1);
		base_offsets[i] = size;
		size += baseinfo.size;
		if (baseinfo.alignment > alignment)
			alignment = baseinfo.alignment;
	}

	std::pmr::vector<std::size_t> field_offsets(temporary_resource.get());
	field_offsets.resize(num_field);

	for (size_t i = 0; i < num_field; ++i) {
		const auto& field_type = field_types[i];
		auto fttarget = typeinfos.find(field_type);
		if (fttarget == typeinfos.end())
			return {};
		const auto& ftinfo = fttarget->second;
		if (!ftinfo.is_trivial)
			is_trivial = false;
		assert(ftinfo.alignment > 0 && (ftinfo.alignment & (ftinfo.alignment - 1)) == 0);
		size = (size + (ftinfo.alignment - 1)) & ~(ftinfo.alignment - 1);
		field_offsets[i] = size;
		size += ftinfo.size;
		if (ftinfo.alignment > alignment)
			alignment = ftinfo.alignment;
	}
	
	size = (size + (alignment - 1)) & ~(alignment - 1);

	Type newtype = RegisterType(type, size, alignment, false, is_trivial);
	for (size_t i = 0; i < bases.size(); i++) {
		AddBase(
			type,
			bases[i],
			BaseInfo{ {
				[offset = base_offsets[i]] (void* derived) {
					return forward_offset(derived, offset);
				},
				[offset = base_offsets[i]](void* base) {
					return backward_offset(base, offset);
				}
			} }
		);
	}

	for (size_t i = 0; i < num_field; ++i) {
		AddField(
			type,
			field_names[i],
			FieldInfo{ { field_types[i], field_offsets[i] } }
		);
	}

	return newtype;
}

Name ReflMngr::AddField(Type type, Name field_name, FieldInfo fieldinfo) {
	auto* typeinfo = GetTypeInfo(type);
	if (!typeinfo) {
		assert(false);
		return {};
	}
	auto ftarget = typeinfo->fieldinfos.find(field_name);
	if (ftarget != typeinfo->fieldinfos.end())
		return {};

	Name new_field_name = { nregistry.Register(field_name.GetID(), field_name.GetView()), field_name.GetID() };
	typeinfo->fieldinfos.emplace_hint(ftarget, new_field_name, std::move(fieldinfo));

	return new_field_name;
}

Name ReflMngr::AddMethod(Type type, Name method_name, MethodInfo methodinfo) {
	auto* typeinfo = GetTypeInfo(type);
	if (!typeinfo) {
		assert(false);
		return {};
	}
	
	auto [begin_iter, end_iter] = typeinfo->methodinfos.equal_range(method_name);
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (!iter->second.methodptr.IsDistinguishableWith(methodinfo.methodptr))
			return {};
	}
	Name new_method_name = { nregistry.Register(method_name.GetID(), method_name.GetView()), method_name.GetID() };
	typeinfo->methodinfos.emplace(new_method_name, std::move(methodinfo));
	return new_method_name;
}

Name ReflMngr::AddTrivialDefaultConstructor(Type type) {
	auto target = typeinfos.find(type);
	if (target == typeinfos.end()) {
		assert(false);
		return {};
	}
	if (target->second.is_polymorphic || ContainsVirtualBase(type))
		return {};
	for (const auto& [basetype, baseinfo] : target->second.baseinfos) {
		assert(!baseinfo.IsPolymorphic()); // type isn't polymorphic => bases aren't polymorphic
		if (baseinfo.IsVirtual())
			return {};
	}
	return AddMethod(
		type,
		NameIDRegistry::Meta::ctor,
		MethodInfo{ {
			[](void* obj, void*, ArgsView) {},
			MethodFlag::Variable
		} }
	);
}

Name ReflMngr::AddTrivialCopyConstructor(Type type) {
	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return {};
	auto& typeinfo = target->second;
	return AddMethod(
		type,
		NameIDRegistry::Meta::ctor,
		MethodInfo{ {
			[size = typeinfo.size](void* obj, void*, ArgsView args) {
				memcpy(obj, args[0].GetPtr(), size);
			},
			MethodFlag::Variable,
			{}, // result type
			{ tregistry.RegisterAddConstLValueReference(type) } // paramlist
		} }
	);
}

Name ReflMngr::AddZeroDefaultConstructor(Type type) {
	auto target = typeinfos.find(type);
	if (target == typeinfos.end() || target->second.is_polymorphic || ContainsVirtualBase(type))
		return {};
	for (const auto& [basetype, baseinfo] : target->second.baseinfos) {
		assert(!baseinfo.IsPolymorphic()); // type isn't polymorphic => bases aren't polymorphic
		if (baseinfo.IsVirtual())
			return {};
	}
	return AddMethod(
		type,
		NameIDRegistry::Meta::ctor,
		MethodInfo{ {
			[size = target->second.size](void* obj, void*, ArgsView) {
				std::memset(obj, 0, size);
			},
			MethodFlag::Variable
		} }
	);
}

Name ReflMngr::AddDefaultConstructor(Type type) {
	if (IsConstructible(type))
		return {};

	auto target = typeinfos.find(type);
	if (target == typeinfos.end() || target->second.is_polymorphic || ContainsVirtualBase(type))
		return {};
	const auto& typeinfo = target->second;
	for (const auto& [basetype, baseinfo] : typeinfo.baseinfos) {
		assert(!baseinfo.IsPolymorphic() && !baseinfo.IsVirtual()); // type isn't polymorphic => bases aren't polymorphic
		if (!IsConstructible(basetype))
			return {};
	}
	for (const auto& [fieldname, fieldinfo] : typeinfo.fieldinfos) {
		if (fieldinfo.fieldptr.GetFieldFlag() == FieldFlag::Unowned)
			continue;

		if (!IsConstructible(fieldinfo.fieldptr.GetType()))
			return {};
	}
	const auto& t = target->first;
	return AddMethod(
		type,
		NameIDRegistry::Meta::ctor,
		MethodInfo{ {
			[t](void* obj, void*, ArgsView) {
				const auto& typeinfo = Mngr.typeinfos.at(t);
				for (const auto& [basetype, baseinfo] : typeinfo.baseinfos) {
					void* baseptr = baseinfo.StaticCast_DerivedToBase(obj);
					bool success = Mngr.Construct(ObjectView{ basetype, baseptr });
					assert(success);
				}

				for (const auto& [fieldname, fieldinfo] : typeinfo.fieldinfos) {
					if (fieldinfo.fieldptr.GetFieldFlag() == FieldFlag::Unowned)
						continue;

					if (fieldinfo.fieldptr.GetType().IsPointer())
						buffer_as<void*>(fieldinfo.fieldptr.Var(obj).GetPtr()) = nullptr;
					else
						Mngr.Construct(fieldinfo.fieldptr.Var(obj));
				}
			},
			MethodFlag::Variable
		} }
	);
}

Name ReflMngr::AddDestructor(Type type) {
	if (IsDestructible(type))
		return {};

	auto target = typeinfos.find(type);
	if (target == typeinfos.end() || target->second.is_polymorphic || ContainsVirtualBase(type))
		return {};
	const auto& typeinfo = target->second;
	for (const auto& [basetype, baseinfo] : typeinfo.baseinfos) {
		assert(!baseinfo.IsPolymorphic() && !baseinfo.IsVirtual()); // type isn't polymorphic => bases aren't polymorphic
		if (!IsDestructible(basetype))
			return {};
	}
	for (const auto& [fieldname, fieldinfo] : typeinfo.fieldinfos) {
		if (fieldinfo.fieldptr.GetFieldFlag() == FieldFlag::Unowned)
			continue;

		if (!type.IsReference() && !IsDestructible(fieldinfo.fieldptr.GetType()))
			return {};
	}
	const auto& t = target->first;
	return AddMethod(
		type,
		NameIDRegistry::Meta::dtor,
		MethodInfo{ {
			[t](void* obj, void*, ArgsView) {
				const auto& typeinfo = Mngr.typeinfos.at(t);

				for (const auto& [fieldname, fieldinfo] : typeinfo.fieldinfos) {
					if (fieldinfo.fieldptr.GetFieldFlag() == FieldFlag::Unowned)
						continue;
					Type ftype = fieldinfo.fieldptr.GetType();
					if (ftype.IsReference())
						continue;
					Mngr.Destruct(fieldinfo.fieldptr.Var(obj));
				}

				for (const auto& [basetype, baseinfo] : typeinfo.baseinfos) {
					void* baseptr = baseinfo.StaticCast_DerivedToBase(obj);
					Mngr.Destruct(ObjectView{ basetype, baseptr });
				}
			},
			MethodFlag::Variable
		} }
	);
}

Type ReflMngr::AddBase(Type derived, Type base, BaseInfo baseinfo) {
	auto* typeinfo = GetTypeInfo(derived);
	if (!typeinfo)
		return {};
	auto btarget = typeinfo->baseinfos.find(base.RemoveCVRef());
	if (btarget != typeinfo->baseinfos.end())
		return {};
	Type new_base_type = { tregistry.Register(base.GetID(), base.GetName()), base.GetID() };
	typeinfo->baseinfos.emplace_hint(btarget, new_base_type, std::move(baseinfo));
	return new_base_type;
}

bool ReflMngr::AddTypeAttr(Type type, Attr attr) {
	auto* typeinfo = GetTypeInfo(type);
	if (!typeinfo)
		return false;
	auto& attrs = typeinfo->attrs;
	auto atarget = attrs.find(attr);
	if (atarget != attrs.end())
		return false;
	attrs.emplace_hint(atarget, std::move(attr));
	return true;
}

bool ReflMngr::AddFieldAttr(Type type, Name name, Attr attr) {
	auto* typeinfo = GetTypeInfo(type);
	if (!typeinfo)
		return false;
	auto ftarget = typeinfo->fieldinfos.find(name);
	if (ftarget == typeinfo->fieldinfos.end())
		return false;
	auto& attrs = ftarget->second.attrs;
	auto atarget = attrs.find(attr);
	if (atarget != attrs.end())
		return false;
	attrs.emplace_hint(atarget, std::move(attr));
	return true;
}

bool ReflMngr::AddMethodAttr(Type type, Name name, Attr attr) {
	auto* typeinfo = GetTypeInfo(type);
	if (!typeinfo)
		return false;
	auto mtarget = typeinfo->methodinfos.find(name);
	if (mtarget == typeinfo->methodinfos.end())
		return false;
	auto& attrs = mtarget->second.attrs;
	auto atarget = attrs.find(attr);
	if (atarget != attrs.end())
		return false;
	attrs.emplace_hint(atarget, std::move(attr));
	return true;
}

SharedObject ReflMngr::MMakeShared(Type type, std::pmr::memory_resource* rsrc, ArgsView args) const {
	if (!IsDestructible(type))
		return {};

	ObjectView obj = MNew(type, rsrc, args);

	if (!obj.GetType().Valid())
		return {};

	return { obj, [rsrc, type](void* ptr) {
		Mngr.MDelete({type, ptr}, rsrc);
	} };
}

ObjectView ReflMngr::StaticCast_DerivedToBase(ObjectView obj, Type type) const {
	constexpr auto cast = [](ObjectView obj, Type type) -> ObjectView {
		for (const auto& [typeinfo, base_obj] : ObjectTree{ obj }) {
			if (base_obj.GetType() == type)
				return base_obj;
		}
		return {};
	};

	const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		return cast(obj.RemoveLValueReference(), type).AddLValueReference();
	case CVRefMode::Right:
		return cast(obj.RemoveRValueReference(), type).AddRValueReference();
	case CVRefMode::Const:
		return cast(obj.RemoveConst(), type).AddConst();
	case CVRefMode::ConstLeft:
		return cast(obj.RemoveConstReference(), type).AddConstLValueReference();
	case CVRefMode::ConstRight:
		return cast(obj.RemoveConstReference(), type).AddConstRValueReference();
	default:
		return cast(obj, type);
	}
}

ObjectView ReflMngr::StaticCast_BaseToDerived(ObjectView obj, Type type) const {
	if (obj.GetPtr() == nullptr)
		return { type, nullptr };

	const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		return details::StaticCast_BaseToDerived(obj.RemoveLValueReference(), type).AddLValueReference();
	case CVRefMode::Right:
		return details::StaticCast_BaseToDerived(obj.RemoveRValueReference(), type).AddRValueReference();
	case CVRefMode::Const:
		return details::StaticCast_BaseToDerived(obj.RemoveConst(), type).AddConst();
	case CVRefMode::ConstLeft:
		return details::StaticCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstLValueReference();
	case CVRefMode::ConstRight:
		return details::StaticCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstRValueReference();
	default:
		return details::StaticCast_BaseToDerived(obj, type);
	}
}

ObjectView ReflMngr::DynamicCast_BaseToDerived(ObjectView obj, Type type) const {
	if (obj.GetPtr() == nullptr)
		return { type, nullptr };

	const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		return details::DynamicCast_BaseToDerived(obj.RemoveLValueReference(), type).AddLValueReference();
	case CVRefMode::Right:
		return details::DynamicCast_BaseToDerived(obj.RemoveRValueReference(), type).AddRValueReference();
	case CVRefMode::Const:
		return details::DynamicCast_BaseToDerived(obj.RemoveConst(), type).AddConst();
	case CVRefMode::ConstLeft:
		return details::DynamicCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstLValueReference();
	case CVRefMode::ConstRight:
		return details::DynamicCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstRValueReference();
	default:
		return details::DynamicCast_BaseToDerived(obj, type);
	}
}

ObjectView ReflMngr::StaticCast(ObjectView obj, Type type) const {
	auto ptr_d2b = StaticCast_DerivedToBase(obj, type);
	if (ptr_d2b.GetType())
		return ptr_d2b;

	auto ptr_b2d = StaticCast_BaseToDerived(obj, type);
	if (ptr_b2d.GetType())
		return ptr_b2d;

	return {};
}

ObjectView ReflMngr::DynamicCast(ObjectView obj, Type type) const {
	auto ptr_b2d = DynamicCast_BaseToDerived(obj, type);
	if (ptr_b2d.GetType())
		return ptr_b2d;

	auto ptr_d2b = StaticCast_DerivedToBase(obj, type);
	if (ptr_d2b.GetType())
		return ptr_d2b;

	return {};
}

ObjectView ReflMngr::Var(ObjectView obj, Name field_name, FieldFlag flag) const {
	for (const auto& [name, var] : VarRange{ obj, flag }) {
		if (name == field_name)
			return var;
	}
	return {};
}

ObjectView ReflMngr::Var(ObjectView obj, Type base, Name field_name, FieldFlag flag) const {
	auto base_obj = StaticCast_DerivedToBase(obj, base);
	if (!base_obj.GetType())
		return {};
	return Var(base_obj, field_name);
}

bool ReflMngr::IsCompatible(std::span<const Type> paramTypes, std::span<const Type> argTypes) const {
	if (paramTypes.size() != argTypes.size())
		return false;

	for (size_t i = 0; i < paramTypes.size(); i++) {
		if (paramTypes[i] == argTypes[i] || paramTypes[i].Is<ObjectView>())
			continue;

		const auto& lhs = paramTypes[i];
		const auto& rhs = argTypes[i];

		if (lhs.IsLValueReference()) { // &{T} | &{const{T}}
			const auto unref_lhs = lhs.Name_RemoveLValueReference(); // T | const{T}
			if (type_name_is_const(unref_lhs)) { // &{const{T}}
				if (unref_lhs == rhs.Name_RemoveRValueReference())
					continue; // &{const{T}} <- &&{const{T}} / const{T}

				const auto raw_lhs = type_name_remove_const(unref_lhs); // T

				if (rhs.Is(raw_lhs) || raw_lhs == rhs.Name_RemoveReference())
					continue; // &{const{T}} <- T | &{T} | &&{T}

				if (details::IsRefConstructible(raw_lhs, std::span<const Type>{&rhs, 1}) && IsDestructible(raw_lhs))
					continue; // &{const{T}} <- T{arg}
			}
		}
		else if (lhs.IsRValueReference()) { // &&{T} | &&{const{T}}
			const auto unref_lhs = lhs.RemoveRValueReference(); // T | const{T}

			if (type_name_is_const(unref_lhs)) { // &&{const{T}}
				const auto raw_lhs = type_name_remove_const(unref_lhs); // T

				if (raw_lhs == type_name_remove_const(rhs))
					continue; // &&{const{T}} <- T / const{T}

				if (raw_lhs == rhs.Name_RemoveRValueReference()) // &&{const{T}}
					continue; // &&{const{T}} <- &&{T}

				if (details::IsRefConstructible(raw_lhs, std::span<const Type>{&rhs, 1}))
					continue; // &&{const{T}} <- T{arg}
			}
			else {
				if (rhs.Is(unref_lhs))
					continue; // &&{T} <- T

				if (details::IsRefConstructible(unref_lhs, std::span<const Type>{&rhs, 1}) && IsDestructible(unref_lhs))
					continue; // &&{T} <- T{arg}
			}
		}
		else { // T
			if (lhs.Is(rhs.Name_RemoveRValueReference()))
				continue; // T <- &&{T}

			if (details::IsRefConstructible(lhs, std::span<const Type>{&rhs, 1}) && IsDestructible(lhs))
				continue; // T <- T{arg}
		}

		if (is_pointer_array_compatible(lhs, rhs))
			continue;

		return false;
	}

	return true;
}

Type ReflMngr::IsInvocable(Type type, Name method_name, std::span<const Type> argTypes, MethodFlag flag) const {
	const CVRefMode cvref_mode = type.GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left: [[fallthrough]];
	case CVRefMode::Right:
		type = type.RemoveReference();
		break;
	case CVRefMode::Const: [[fallthrough]];
	case CVRefMode::ConstLeft: [[fallthrough]];
	case CVRefMode::ConstRight:
		type = type.RemoveCVRef();
		flag = enum_remove(flag, MethodFlag::Variable);
		break;
	default:
		break;
	}
	
	auto is_invocable = [&](bool is_priority, MethodFlag filter) -> Type {
		if (!enum_contain_any(flag, filter))
			return {};

		MethodFlag newflag = enum_within(flag, filter);

		for (const auto& [typeinfo, baseobj] : ObjectTree{ type }) {
			if (!typeinfo)
				continue;

			auto [begin_iter, end_iter] = typeinfo->methodinfos.equal_range(method_name);
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (enum_contain_any(newflag, iter->second.methodptr.GetMethodFlag())
					&& (is_priority ? details::IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypes)
						: Mngr.IsCompatible(iter->second.methodptr.GetParamList(), argTypes)))
				{
					return iter->second.methodptr.GetResultType();
				}
			}
		}

		return {};
	};

	if (auto rst = is_invocable(true, MethodFlag::Priority))
		return rst;
	if (auto rst = is_invocable(true, MethodFlag::Const))
		return rst;
	if (auto rst = is_invocable(false, MethodFlag::Priority))
		return rst;
	if (auto rst = is_invocable(false, MethodFlag::Const))
		return rst;

	return {};
}

Type ReflMngr::BInvoke(
	ObjectView obj,
	Name method_name,
	void* result_buffer,
	ArgsView args,
	MethodFlag flag,
	std::pmr::memory_resource* temp_args_rsrc) const
{
	assert(temp_args_rsrc);

	const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left: [[fallthrough]];
	case CVRefMode::Right:
		obj = obj.RemoveReference();
		break;
	case CVRefMode::Const:
		obj = obj.RemoveConst();
		flag = enum_remove(flag, MethodFlag::Variable);
		break;
	case CVRefMode::ConstLeft: [[fallthrough]];
	case CVRefMode::ConstRight:
		obj = obj.RemoveConstReference();
		flag = enum_remove(flag, MethodFlag::Variable);
		break;
	default:
		break;
	}

	if (!obj.GetPtr())
		flag = enum_within(flag, MethodFlag::Static);

	auto binvoke = [&](bool is_priority, MethodFlag filter) -> Type {
		if (!enum_contain_any(flag, filter))
			return {};

		MethodFlag newflag = enum_within(flag, filter);

		for (const auto& [typeinfo, baseobj] : ObjectTree{ obj }) {
			if (!typeinfo)
				continue;

			auto [begin_iter, end_iter] = typeinfo->methodinfos.equal_range(method_name);

			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (!enum_contain_any(newflag, iter->second.methodptr.GetMethodFlag()))
					continue;

				details::NewArgsGuard guard{
					is_priority, temp_args_rsrc,
					iter->second.methodptr.GetParamList(), args
				};
				if (!guard.IsCompatible())
					continue;
				iter->second.methodptr.Invoke(baseobj.GetPtr(), result_buffer, guard.GetArgsView());
				return iter->second.methodptr.GetResultType();
			}
		}
		return {};
	};

	if (auto rst = binvoke(true, MethodFlag::Priority))
		return rst;
	if (auto rst = binvoke(true, MethodFlag::Const))
		return rst;
	if (auto rst = binvoke(false, MethodFlag::Priority))
		return rst;
	if (auto rst = binvoke(false, MethodFlag::Const))
		return rst;

	return {};
}

SharedObject ReflMngr::MInvoke(
	ObjectView obj,
	Name method_name,
	std::pmr::memory_resource* rst_rsrc,
	ArgsView args,
	MethodFlag flag,
	std::pmr::memory_resource* temp_args_rsrc) const
{
	assert(rst_rsrc);
	assert(temp_args_rsrc);

	const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left: [[fallthrough]];
	case CVRefMode::Right:
		obj = obj.RemoveReference();
		break;
	case CVRefMode::Const:
		obj = obj.RemoveConst();
		flag = enum_remove(flag, MethodFlag::Variable);
		break;
	case CVRefMode::ConstLeft: [[fallthrough]];
	case CVRefMode::ConstRight:
		obj = obj.RemoveConstReference();
		flag = enum_remove(flag, MethodFlag::Variable);
		break;
	default:
		break;
	}

	if (!obj.GetPtr())
		flag = enum_within(flag, MethodFlag::Static);

	auto minvoke = [&](bool is_priority, MethodFlag filter) -> SharedObject {
		if (!enum_contain_any(flag, filter))
			return {};

		MethodFlag newflag = enum_within(flag, filter);

		for (const auto& [typeinfo, baseobj] : ObjectTree{ obj }) {
			if (!typeinfo)
				continue;

			auto [begin_iter, end_iter] = typeinfo->methodinfos.equal_range(method_name);

			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (!enum_contain_any(newflag, iter->second.methodptr.GetMethodFlag()))
					continue;

				details::NewArgsGuard guard{
					is_priority, temp_args_rsrc,
					iter->second.methodptr.GetParamList(), args
				};

				if (!guard.IsCompatible())
					continue;

				const auto& methodptr = iter->second.methodptr;
				const auto& rst_type = methodptr.GetResultType();

				if (rst_type.Is<void>()) {
					iter->second.methodptr.Invoke(baseobj.GetPtr(), nullptr, guard.GetArgsView());
					return SharedObject{ Type_of<void> };
				}
				else if (rst_type.IsReference()) {
					std::aligned_storage_t<sizeof(void*)> buffer;
					iter->second.methodptr.Invoke(baseobj.GetPtr(), &buffer, guard.GetArgsView());
					return { rst_type, buffer_as<void*>(&buffer) };
				}
				else if (rst_type.Is<ObjectView>()) {
					std::aligned_storage_t<sizeof(ObjectView)> buffer;
					iter->second.methodptr.Invoke(baseobj.GetPtr(), &buffer, guard.GetArgsView());
					return SharedObject{ buffer_as<ObjectView>(&buffer) };
				}
				else if (rst_type.Is<SharedObject>()) {
					SharedObject buffer;
					iter->second.methodptr.Invoke(baseobj.GetPtr(), &buffer, guard.GetArgsView());
					return buffer;
				}
				else {
					if (!Mngr.IsDestructible(rst_type))
						return {};
					auto* result_typeinfo = Mngr.GetTypeInfo(rst_type);
					if (!result_typeinfo)
						return {};
					void* result_buffer = rst_rsrc->allocate(result_typeinfo->size, result_typeinfo->alignment);
					iter->second.methodptr.Invoke(baseobj.GetPtr(), result_buffer, guard.GetArgsView());
					return {
						{rst_type, result_buffer},
						[rst_type, rst_rsrc](void* ptr) { Mngr.MDelete({ rst_type, ptr }, rst_rsrc); }
					};
				}
			}
		}
		return {};
	};

	if (auto rst = minvoke(true, MethodFlag::Priority); rst.GetType())
		return rst;
	if (auto rst = minvoke(true, MethodFlag::Const); rst.GetType())
		return rst;
	if (auto rst = minvoke(false, MethodFlag::Priority); rst.GetType())
		return rst;
	if (auto rst = minvoke(false, MethodFlag::Const); rst.GetType())
		return rst;

	return {};
}

ObjectView ReflMngr::MNew(Type type, std::pmr::memory_resource* rsrc, ArgsView args) const {
	assert(rsrc);

	if (!IsConstructible(type, args.Types()))
		return {};

	const auto& typeinfo = typeinfos.at(type);

	void* buffer = rsrc->allocate(std::max<std::size_t>(1, typeinfo.size), typeinfo.alignment);

	if (!buffer)
		return {};

	ObjectView obj{ type, buffer };
	bool success = Construct(obj, args);
	assert(success);

	return obj;
}

bool ReflMngr::MDelete(ObjectView obj, std::pmr::memory_resource* rsrc) const {
	assert(rsrc);

	Destruct(obj);

	const auto& typeinfo = typeinfos.at(obj.GetType());

	rsrc->deallocate(obj.GetPtr(), std::max<std::size_t>(1, typeinfo.size), typeinfo.alignment);

	return true;
}

ObjectView ReflMngr::New(Type type, ArgsView args) const {
	return MNew(type, object_resource.get(), args);
}

bool ReflMngr::Delete(ObjectView obj) const {
	return MDelete(obj, object_resource.get());
}

SharedObject ReflMngr::MakeShared(Type type, ArgsView args) const {
	return MMakeShared(type, object_resource.get(), args);
}

bool ReflMngr::IsConstructible(Type type, std::span<const Type> argTypes) const {
	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;

	if (typeinfo.is_trivial && (
		argTypes.empty() // default ctor
		|| argTypes.size() == 1 && argTypes.front().RemoveCVRef() == type // const/ref ctor
	))
	{ return true; }

	auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::ctor);
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (IsCompatible(iter->second.methodptr.GetParamList(), argTypes))
			return true;
	}
	return false;
}

bool ReflMngr::IsCopyConstructible(Type type) const {
	const Type clref_type = tregistry.RegisterAddConstLValueReference(type);
	return details::IsRefConstructible(type, std::span<const Type>{&clref_type, 1});
}

bool ReflMngr::IsMoveConstructible(Type type) const {
	const Type rref_type = tregistry.RegisterAddRValueReference(type);
	return details::IsRefConstructible(type, std::span<const Type>{&rref_type, 1});
}

bool ReflMngr::IsDestructible(Type type) const {
	assert(type.GetCVRefMode() == CVRefMode::None);

	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	if (typeinfo.is_trivial)
		return true;
	auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::dtor);
	if (begin_iter == end_iter)
		return true;
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Variable
			&& IsCompatible(iter->second.methodptr.GetParamList(), {}))
			return true;
	}
	return false;
}

bool ReflMngr::Construct(ObjectView obj, ArgsView args) const {
	auto target = typeinfos.find(obj.GetType());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	if (args.Types().empty() && typeinfo.is_trivial)
		return true; // trivial ctor
	auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::ctor);
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Variable) {
			details::NewArgsGuard guard{
				false, temporary_resource.get(),
				iter->second.methodptr.GetParamList(), args
			};
			if (!guard.IsCompatible())
				continue;
			iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, guard.GetArgsView());
			return true;
		}
	}
	return false;
}

bool ReflMngr::Destruct(ObjectView obj) const {
	auto target = typeinfos.find(obj.GetType());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	if (typeinfo.is_trivial)
		return true;// trivial ctor
	auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::dtor);
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Variable
			&& IsCompatible(iter->second.methodptr.GetParamList(), {}))
		{
			iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, {});
			return true;
		}
	}
	return false;
}
