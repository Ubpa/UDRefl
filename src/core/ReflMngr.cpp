#include <UDRefl/ReflMngr.h>

#include "InvokeUtil.h"

#include "ReflMngrInitUtil/ReflMngrInitUtil.h"

#include <string>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

ReflMngr* Ubpa::UDRefl::Mngr = &ReflMngr::Instance();
const ObjectView Ubpa::UDRefl::MngrView = { Type_of<ReflMngr>, &ReflMngr::Instance() };

namespace Ubpa::UDRefl::details {
	DeleteFunc GenerateDeleteFunc(Type type, std::pmr::memory_resource* result_rsrc, size_t size, size_t alignment) {
		return [type, result_rsrc, size, alignment](void* ptr) {
			Mngr->Destruct(ObjectView{ type, ptr });
			result_rsrc->deallocate(ptr, size, alignment);
		};
	}

	static ObjectView StaticCast_DerivedToBase(ObjectView obj, Type type) {
		assert(obj.GetType().GetCVRefMode() == CVRefMode::None);

		if (obj.GetType() == type)
			return obj;

		auto target = Mngr->typeinfos.find(obj.GetType());
		if (target == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = target->second;

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			auto ptr = StaticCast_DerivedToBase(ObjectView{ base, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, type);
			if (ptr.GetType())
				return ptr;
		}

		return {};
	}

	static ObjectView StaticCast_BaseToDerived(ObjectView obj, Type type) {
		assert(obj.GetType().GetCVRefMode() == CVRefMode::None);

		if (obj.GetType() == type)
			return obj;

		auto target = Mngr->typeinfos.find(type);
		if (target == Mngr->typeinfos.end())
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

		auto target = Mngr->typeinfos.find(obj.GetType());
		if (target == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = target->second;

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			auto ptr = DynamicCast_BaseToDerived(ObjectView{ base, baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) }, type);
			if (ptr.GetType())
				return { base, baseinfo.IsPolymorphic() ? baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) : nullptr };
		}

		return {};
	}

	static ObjectView Var(ObjectView obj, Name field_name, FieldFlag flag) {
		assert(obj.GetType().GetCVRefMode() == CVRefMode::None);

		auto ttarget = Mngr->typeinfos.find(obj.GetType());
		if (ttarget == Mngr->typeinfos.end())
			return {};

		auto& typeinfo = ttarget->second;

		auto ftarget = typeinfo.fieldinfos.find(field_name);
		if (ftarget != typeinfo.fieldinfos.end() && enum_contain(flag, ftarget->second.fieldptr.GetFieldFlag()))
			return ftarget->second.fieldptr.Var(obj.GetPtr());

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			auto bptr = Var(ObjectView{ base, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, field_name, flag);
			if (bptr.GetType())
				return bptr;
		}

		return {};
	}

	static Type IsInvocable(
		bool is_priority,
		Type type,
		Name method_name,
		std::span<const Type> argTypes,
		MethodFlag flag)
	{
		assert(type.GetCVRefMode() == CVRefMode::None);
		auto typetarget = Mngr->typeinfos.find(type);

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;
		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(method_name);

		// 1. object variable
		if (enum_contain(flag, MethodFlag::Variable)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Variable
					&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypes)
						: Mngr->IsCompatible(iter->second.methodptr.GetParamList(), argTypes)))
				{
					return iter->second.methodptr.GetResultType();
				}
			}
		}

		// 2. object const and static
		if(enum_contain(flag, MethodFlag::Const | MethodFlag::Static)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (iter->second.methodptr.GetMethodFlag() != MethodFlag::Variable && enum_contain_any(flag, iter->second.methodptr.GetMethodFlag())
					&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypes)
						: Mngr->IsCompatible(iter->second.methodptr.GetParamList(), argTypes)))
				{
					return iter->second.methodptr.GetResultType();
				}
			}
		}

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			if (auto rst = IsInvocable(is_priority, base, method_name, argTypes, flag))
				return rst;
		}

		return {};
	}

	static Type BInvoke(
		bool is_priority,
		std::pmr::memory_resource* args_rsrc,
		ObjectView obj,
		Name method_name,
		void* result_buffer,
		std::span<const Type> argTypes,
		ArgPtrBuffer argptr_buffer,
		MethodFlag flag)
	{
		assert(obj.GetType().GetCVRefMode() == CVRefMode::None);

		auto typetarget = Mngr->typeinfos.find(obj.GetType());

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(method_name);

		if (enum_contain_any(flag, MethodFlag::Priority)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (enum_contain(flag, iter->second.methodptr.GetMethodFlag())) {
					NewArgsGuard guard{
						is_priority, args_rsrc,
						iter->second.methodptr.GetParamList(), argTypes, argptr_buffer
					};
					if (!guard.IsCompatible())
						continue;
					iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, guard.GetArgPtrBuffer());
					return iter->second.methodptr.GetResultType();
				}
			}
		}
		if (enum_contain(flag, MethodFlag::Const)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Const) {
					NewArgsGuard guard{
						is_priority, args_rsrc,
						iter->second.methodptr.GetParamList(), argTypes, argptr_buffer
					};
					if (!guard.IsCompatible())
						continue;
					iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), result_buffer, guard.GetArgPtrBuffer());
					return iter->second.methodptr.GetResultType();
				}
			}
		}


		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			auto rst = BInvoke(
				is_priority, args_rsrc,
				ObjectView{ base, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
				method_name, result_buffer, argTypes, argptr_buffer,
				flag
			);
			if (rst)
				return rst;
		}

		return {};
	}

	static SharedObject MInvoke(
		bool is_priority,
		std::pmr::memory_resource* args_rsrc,
		ObjectView obj,
		Name method_name,
		std::pmr::memory_resource* rst_rsrc,
		std::span<const Type> argTypes,
		ArgPtrBuffer argptr_buffer,
		MethodFlag flag)
	{
		assert(args_rsrc);
		assert(rst_rsrc);
		assert(obj.GetType().GetCVRefMode() == CVRefMode::None);
		auto typetarget = Mngr->typeinfos.find(obj.GetType());

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(method_name);

		if (enum_contain(flag, MethodFlag::Priority)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (enum_contain(flag, iter->second.methodptr.GetMethodFlag())) {
					NewArgsGuard guard{
						is_priority, args_rsrc,
						iter->second.methodptr.GetParamList(), argTypes, argptr_buffer
					};

					if (!guard.IsCompatible())
						continue;

					const auto& methodptr = iter->second.methodptr;
					const auto& rst_type = methodptr.GetResultType();

					if (rst_type.Is<void>()) {
						iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, argptr_buffer);
						return SharedObject{ Type_of<void> };
					}
					else if (rst_type.IsReference()) {
						std::aligned_storage_t<sizeof(void*)> buffer;
						iter->second.methodptr.Invoke(obj.GetPtr(), &buffer, guard.GetArgPtrBuffer());
						return { rst_type, buffer_as<void*>(&buffer) };
					}
					else if (rst_type.Is<ObjectView>()) {
						std::aligned_storage_t<sizeof(ObjectView)> buffer;
						iter->second.methodptr.Invoke(obj.GetPtr(), &buffer, argptr_buffer);
						return SharedObject{ buffer_as<ObjectView>(&buffer) };
					}
					else if (rst_type.Is<SharedObject>()) {
						SharedObject buffer;
						iter->second.methodptr.Invoke(obj.GetPtr(), &buffer, argptr_buffer);
						return buffer;
					}
					else {
						auto* result_typeinfo = Mngr->GetTypeInfo(rst_type);
						if (!result_typeinfo)
							return {};
						void* result_buffer = rst_rsrc->allocate(result_typeinfo->size, result_typeinfo->alignment);
						iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, guard.GetArgPtrBuffer());
						return {
							{rst_type, result_buffer},
							GenerateDeleteFunc(iter->second.methodptr.GetResultType(), rst_rsrc, result_typeinfo->size, result_typeinfo->alignment)
						};
					}
				}
			}
		}

		if (enum_contain_any(flag, MethodFlag::Const)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Const) {
					NewArgsGuard guard{
						is_priority, args_rsrc,
						iter->second.methodptr.GetParamList(), argTypes, argptr_buffer
					};

					if (!guard.IsCompatible())
						continue;

					const auto& methodptr = iter->second.methodptr;
					const auto& rst_type = methodptr.GetResultType();

					if (rst_type.Is<void>()) {
						iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), nullptr, argptr_buffer);
						return SharedObject{ rst_type };
					}
					else if (rst_type.IsReference()) {
						std::aligned_storage_t<sizeof(void*)> buffer;
						iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), &buffer, guard.GetArgPtrBuffer());
						return { rst_type, buffer_as<void*>(&buffer) };
					}
					else if (rst_type.Is<ObjectView>()) {
						std::aligned_storage_t<sizeof(ObjectView)> buffer;
						iter->second.methodptr.Invoke(obj.GetPtr(), &buffer, argptr_buffer);
						return SharedObject{ buffer_as<ObjectView>(&buffer) };
					}
					else if (rst_type.Is<SharedObject>()) {
						SharedObject buffer;
						iter->second.methodptr.Invoke(obj.GetPtr(), &buffer, argptr_buffer);
						return buffer;
					}
					else {
						auto* result_typeinfo = Mngr->GetTypeInfo(rst_type);
						if (!result_typeinfo)
							return {};
						void* result_buffer = rst_rsrc->allocate(result_typeinfo->size, result_typeinfo->alignment);
						iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), result_buffer, guard.GetArgPtrBuffer());
						return {
							{rst_type, result_buffer},
							GenerateDeleteFunc(iter->second.methodptr.GetResultType(), rst_rsrc, result_typeinfo->size, result_typeinfo->alignment)
						};
					}
				}
			}
		}

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			auto rst = MInvoke(
				is_priority, args_rsrc,
				ObjectView{ base, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
				method_name, rst_rsrc, argTypes, argptr_buffer,
				flag
			);
			if (rst.GetType())
				return rst;
		}

		return {};
	}

	static bool ForEachTypeInfo(
		Type type,
		const std::function<bool(InfoTypePair)>& func,
		std::set<TypeID>& visitedVBs)
	{
		assert(type.GetCVRefMode() == CVRefMode::None);
		auto target = Mngr->typeinfos.find(type);

		if (target == Mngr->typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		if (!func({ type, &typeinfo }))
			return false;

		for (auto& [base, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(base) != visitedVBs.end())
					continue;
				visitedVBs.insert(base);
			}

			if (!ForEachTypeInfo(base, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachVar(
		ObjectView obj,
		const std::function<bool(InfoTypePair, InfoFieldPair, ObjectView)>& func,
		FieldFlag flag,
		std::set<TypeID>& visitedVBs)
	{
		assert(obj.GetType().GetCVRefMode() == CVRefMode::None);

		auto target = Mngr->typeinfos.find(obj.GetType());

		if (target == Mngr->typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [field, fieldInfo] : typeinfo.fieldinfos) {
			if (!enum_contain(flag, fieldInfo.fieldptr.GetFieldFlag()))
				continue;

			if (!func({ obj.GetType(), &typeinfo }, { field, &fieldInfo }, fieldInfo.fieldptr.Var(obj.GetPtr())))
				return false;
		}

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(base) != visitedVBs.end())
					continue;
				visitedVBs.insert(base);
			}

			if (!ForEachVar(ObjectView{ base, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, func, flag, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ContainsField(Type type, Name field_name, FieldFlag flag) {
		assert(type.GetCVRefMode() == CVRefMode::None);

		auto ttarget = Mngr->typeinfos.find(type);
		if (ttarget == Mngr->typeinfos.end())
			return false;

		auto& typeinfo = ttarget->second;

		auto ftarget = typeinfo.fieldinfos.find(field_name);
		if (ftarget != typeinfo.fieldinfos.end() && enum_contain(flag, ftarget->second.fieldptr.GetFieldFlag()))
			return true;

		for (const auto& [basetype, baseinfo] : typeinfo.baseinfos) {
			if (ContainsField(basetype, field_name, flag))
				return true;
		}
		return false;
	}

	static bool ContainsMethod(Type type, Name method_name, MethodFlag flag) {
		assert(type.GetCVRefMode() == CVRefMode::None);

		auto ttarget = Mngr->typeinfos.find(type);
		if (ttarget == Mngr->typeinfos.end())
			return false;

		auto& typeinfo = ttarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(method_name);
		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (enum_contain(flag, iter->second.methodptr.GetMethodFlag()))
				return true;
		}

		for (const auto& [basetype, baseinfo] : typeinfo.baseinfos) {
			if (ContainsMethod(basetype, method_name, flag))
				return true;
		}

		return false;
	}
}

ReflMngr::ReflMngr() {
	RegisterType(GlobalType, 0, 0);

	details::ReflMngrInitUtil_0(*this);
	details::ReflMngrInitUtil_1(*this);
	details::ReflMngrInitUtil_2(*this);
	details::ReflMngrInitUtil_3(*this);
	details::ReflMngrInitUtil_4(*this);
	details::ReflMngrInitUtil_5(*this);
	details::ReflMngrInitUtil_6(*this);
}

TypeInfo* ReflMngr::GetTypeInfo(Type type) const {
	auto target = typeinfos.find(type.RemoveCVRef());
	if (target == typeinfos.end())
		return nullptr;
	return const_cast<TypeInfo*>(&target->second);
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

	temporary_resource.release();
	object_resource.release();
}

ReflMngr::~ReflMngr() {
	Clear();
}

Type ReflMngr::RegisterType(Type type, size_t size, size_t alignment) {
	auto target = typeinfos.find(type.RemoveCVRef());
	if (target != typeinfos.end())
		return {};
	Type new_type = { tregistry.Register(type.GetID(), type.GetName()),type.GetID() };
	typeinfos.emplace_hint(target, new_type, TypeInfo{ size,alignment });
	return new_type;
}

Name ReflMngr::AddField(Type type, Name field_name, FieldInfo fieldinfo) {
	auto* typeinfo = GetTypeInfo(type);
	if (!typeinfo)
		return {};
	auto ftarget = typeinfo->fieldinfos.find(field_name);
	if (ftarget != typeinfo->fieldinfos.end())
		return {};

	Name new_field_name = { nregistry.Register(field_name.GetID(), field_name.GetView()), field_name.GetID() };
	typeinfo->fieldinfos.emplace_hint(ftarget, new_field_name, std::move(fieldinfo));

	return new_field_name;
}

Name ReflMngr::AddMethod(Type type, Name method_name, MethodInfo methodinfo) {
	auto* typeinfo = GetTypeInfo(type);
	if (!typeinfo)
		return {};
	
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
	return AddMethod(
		type,
		NameIDRegistry::Meta::ctor,
		MethodInfo{ {[](void*, void*, ArgsView) {}, MethodFlag::Variable} }
	);
}

Name ReflMngr::AddTrivialCopyConstructor(Type type) {
	auto* typeinfo = GetTypeInfo(type);
	if (!typeinfo)
		return {};
	std::size_t size = typeinfo->size;
	return AddMethod(
		type,
		NameIDRegistry::Meta::ctor,
		MethodInfo{ {
			[size](void* obj, void*, ArgsView args) {
				memcpy(obj, args.At(0).GetPtr(), size);
			},
			MethodFlag::Variable,
			{}, // result type
			{ tregistry.RegisterAddConstLValueReference(type) } // paramlist
		} }
	);
}

Name ReflMngr::AddZeroConstructor(Type type) {
	auto* typeinfo = GetTypeInfo(type);
	if (!typeinfo)
		return {};
	std::size_t size;
	return AddMethod(
		type,
		NameIDRegistry::Meta::ctor,
		MethodInfo{ {[size](void* obj, void*, ArgsView) {std::memset(obj,0,size); }, MethodFlag::Variable} }
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

SharedObject ReflMngr::MMakeShared(Type type, std::pmr::memory_resource* rsrc, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	ObjectView obj = MNew(type, rsrc, argTypes, argptr_buffer);

	if (!obj.GetType().Valid())
		return {};

	return { obj, [rsrc, type](void* ptr) {
		Mngr->MDelete({type, ptr}, rsrc);
	} };
}

ObjectView ReflMngr::StaticCast_DerivedToBase(ObjectView obj, Type type) const {
	if (obj.GetPtr() == nullptr)
		return { type, nullptr };

	const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		return details::StaticCast_DerivedToBase(obj.RemoveLValueReference(), type).AddLValueReference();
	case CVRefMode::Right:
		return details::StaticCast_DerivedToBase(obj.RemoveRValueReference(), type).AddRValueReference();
	case CVRefMode::Const:
		return details::StaticCast_DerivedToBase(obj.RemoveConst(), type).AddConst();
	case CVRefMode::ConstLeft:
		return details::StaticCast_DerivedToBase(obj.RemoveConstReference(), type).AddConstLValueReference();
	case CVRefMode::ConstRight:
		return details::StaticCast_DerivedToBase(obj.RemoveConstReference(), type).AddConstRValueReference();
	default:
		return details::StaticCast_DerivedToBase(obj, type);
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
	if (!obj.GetPtr())
		flag = enum_within(flag, FieldFlag::Unowned);

	const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		return details::Var(obj.RemoveLValueReference(), field_name, flag).AddLValueReference();
	case CVRefMode::Right:
		return details::Var(obj.RemoveRValueReference(), field_name, flag).AddRValueReference();
	case CVRefMode::Const:
		return details::Var(obj.RemoveConst(), field_name, flag).AddConst();
	case CVRefMode::ConstLeft:
		return details::Var(obj.RemoveConstReference(), field_name, flag).AddConstLValueReference();
	case CVRefMode::ConstRight:
		return details::Var(obj.RemoveConstReference(), field_name, flag).AddConstRValueReference();
	default:
		return details::Var(obj, field_name, flag);
	}
}

ObjectView ReflMngr::Var(ObjectView obj, Type base, Name field_name, FieldFlag flag) const {
	auto base_obj = StaticCast_DerivedToBase(obj, base);
	if (!base_obj.GetType())
		return {};
	return Var(base_obj, field_name);
}

bool ReflMngr::IsCompatible(std::span<const Type> params, std::span<const Type> argTypes) const {
	if (params.size() != argTypes.size())
		return false;

	for (size_t i = 0; i < params.size(); i++) {
		if (params[i] == argTypes[i])
			continue;

		const auto& lhs = params[i];
		const auto& rhs = argTypes[i];

		if (lhs.IsLValueReference()) { // &{T} | &{const{T}}
			const auto unref_lhs = lhs.Name_RemoveLValueReference(); // T | const{T}
			if (type_name_is_const(unref_lhs)) { // &{const{T}}
				if (unref_lhs == rhs.Name_RemoveRValueReference())
					continue; // &{const{T}} <- &&{const{T}} / const{T}

				const auto raw_lhs = type_name_remove_const(unref_lhs); // T

				if (rhs.Is(raw_lhs) || raw_lhs == rhs.Name_RemoveReference())
					continue; // &{const{T}} <- T | &{T} | &&{T}

				if (details::IsRefConstructible(raw_lhs, std::span<const Type>{&rhs, 1}))
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

				if (details::IsRefConstructible(unref_lhs, std::span<const Type>{&rhs, 1}))
					continue; // &&{T} <- T{arg}
			}
		}
		else { // T
			if (lhs.Is(rhs.Name_RemoveRValueReference()))
				continue; // T <- &&{T}

			if (details::IsRefConstructible(lhs, std::span<const Type>{&rhs, 1}))
				continue; // T <- T{arg}
		}

		if (!(lhs.IsLValueReference() && !lhs.IsReadOnly()) && details::IsPointerAndArrayCompatible(lhs.Name_RemoveCVRef(), rhs.Name_RemoveCVRef()))
			continue;

		return false;
	}

	return true;
}

Type ReflMngr::IsInvocable(Type type, Name method_name, std::span<const Type> argTypes, MethodFlag flag) const {
	Type rawtype;
	MethodFlag newflag;
	const CVRefMode cvref_mode = type.GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left: [[fallthrough]];
	case CVRefMode::Right:
		rawtype = type.RemoveReference();
		newflag = flag;
		break;
	case CVRefMode::Const: [[fallthrough]];
	case CVRefMode::ConstLeft: [[fallthrough]];
	case CVRefMode::ConstRight:
		rawtype = type.RemoveCVRef();
		newflag = enum_remove(flag, MethodFlag::Variable);
		break;
	default:
		rawtype = type;
		newflag = flag;
		break;
	}

	if (auto priority_rst = details::IsInvocable(true, rawtype, method_name, argTypes, newflag))
		return priority_rst;

	return details::IsInvocable(false, rawtype, method_name, argTypes, newflag);
}

Type ReflMngr::BInvoke(
	ObjectView obj,
	Name method_name,
	void* result_buffer,
	std::span<const Type> argTypes,
	ArgPtrBuffer argptr_buffer,
	MethodFlag flag,
	std::pmr::memory_resource* temp_args_rsrc) const
{
	ObjectView rawObj;
	const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left: [[fallthrough]];
	case CVRefMode::Right:
		rawObj = obj.RemoveReference();
		break;
	case CVRefMode::Const:
		rawObj = obj.RemoveConst();
		flag = enum_remove(flag, MethodFlag::Variable);
		break;
	case CVRefMode::ConstLeft: [[fallthrough]];
	case CVRefMode::ConstRight:
		rawObj = obj.RemoveConstReference();
		flag = enum_remove(flag, MethodFlag::Variable);
		break;
	default:
		rawObj = obj;
		break;
	}

	if (!obj.GetPtr())
		flag = enum_within(flag, MethodFlag::Static);

	if (auto priority_rst = details::BInvoke(true, temp_args_rsrc, rawObj, method_name, result_buffer, argTypes, argptr_buffer, flag))
		return priority_rst;

	return details::BInvoke(false, temp_args_rsrc, rawObj, method_name, result_buffer, argTypes, argptr_buffer, flag);
}

SharedObject ReflMngr::MInvoke(
	ObjectView obj,
	Name method_name,
	std::pmr::memory_resource* rst_rsrc,
	std::pmr::memory_resource* temp_args_rsrc,
	std::span<const Type> argTypes,
	ArgPtrBuffer argptr_buffer,
	MethodFlag flag) const
{
	assert(rst_rsrc);
	assert(temp_args_rsrc);

	ObjectView rawObj;
	const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left: [[fallthrough]];
	case CVRefMode::Right:
		rawObj = obj.RemoveReference();
		break;
	case CVRefMode::Const:
		rawObj = obj.RemoveConst();
		flag = enum_remove(flag, MethodFlag::Variable);
		break;
	case CVRefMode::ConstLeft: [[fallthrough]];
	case CVRefMode::ConstRight:
		rawObj = obj.RemoveConstReference();
		flag = enum_remove(flag, MethodFlag::Variable);
		break;
	default:
		rawObj = obj;
		break;
	}

	if (!obj.GetPtr())
		flag = enum_within(flag, MethodFlag::Static);

	if (auto priority_rst = details::MInvoke(true, temp_args_rsrc, rawObj, method_name, rst_rsrc, argTypes, argptr_buffer, flag); priority_rst.GetType().Valid())
		return priority_rst;

	return details::MInvoke(false, temp_args_rsrc, rawObj, method_name, rst_rsrc, argTypes, argptr_buffer, flag);
}

ObjectView ReflMngr::MNew(Type type, std::pmr::memory_resource* rsrc, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	assert(rsrc);

	if (!IsConstructible(type, argTypes))
		return {};

	const auto& typeinfo = typeinfos.at(type);

	void* buffer = rsrc->allocate(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return {};

	ObjectView obj{ type, buffer };
	bool success = Construct(obj, argTypes, argptr_buffer);
	assert(success);

	return obj;
}

bool ReflMngr::MDelete(ObjectView obj, std::pmr::memory_resource* rsrc) const {
	assert(rsrc);

	Destruct(obj);

	const auto& typeinfo = typeinfos.at(obj.GetType());

	rsrc->deallocate(obj.GetPtr(), typeinfo.size, typeinfo.alignment);

	return true;
}

ObjectView ReflMngr::New(Type type, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	return MNew(type, &object_resource, argTypes, argptr_buffer);
}

bool ReflMngr::Delete(ObjectView obj) const {
	return MDelete(obj, &object_resource);
}

SharedObject ReflMngr::MakeShared(Type type, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	return MMakeShared(type, &object_resource, argTypes, argptr_buffer);
}

bool ReflMngr::IsConstructible(Type type, std::span<const Type> argTypes) const {
	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::ctor);
	if (begin_iter == end_iter && argTypes.empty())
		return true;
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

	auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::dtor);
	if (begin_iter == end_iter)
		return true;
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (iter->second.methodptr.GetMethodFlag() != MethodFlag::Variable
			&& IsCompatible(iter->second.methodptr.GetParamList(), {}))
			return true;
	}
	return false;
}

bool ReflMngr::Construct(ObjectView obj, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	auto target = typeinfos.find(obj.GetType());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::ctor);
	if (begin_iter == end_iter && argTypes.empty())
		return true;// trivial ctor
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Variable) {
			details::NewArgsGuard guard{
				false, &temporary_resource,
				iter->second.methodptr.GetParamList(), argTypes, argptr_buffer
			};
			if (!guard.IsCompatible())
				continue;
			iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, guard.GetArgPtrBuffer());
			return true;
		}
	}
	return false;
}

void ReflMngr::Destruct(ObjectView obj) const {
	auto target = typeinfos.find(obj.GetType());
	if (target == typeinfos.end())
		return;
	const auto& typeinfo = target->second;
	auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::dtor);
	if (begin_iter == end_iter)
		return;// trivial dtor
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Const
			&& IsCompatible(iter->second.methodptr.GetParamList(), {}))
		{
			iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, {});
			return;
		}
	}
}

void ReflMngr::ForEachTypeInfo(Type type, const std::function<bool(InfoTypePair)>& func) const {
	std::set<TypeID> visitedVBs;
	details::ForEachTypeInfo(type.RemoveCVRef(), func, visitedVBs);
}

void ReflMngr::ForEachField(
	Type type,
	const std::function<bool(InfoTypePair, InfoFieldPair)>& func,
	FieldFlag flag) const
{
	ForEachTypeInfo(type, [flag, &func](InfoTypePair type) {
		for (auto& [field, fieldInfo] : type.info->fieldinfos) {
			if (!enum_contain(flag, fieldInfo.fieldptr.GetFieldFlag()))
				continue;

			if (!func(type, { field, &fieldInfo }))
				return false;
		}
		return true;
	});
}

void ReflMngr::ForEachMethod(
	Type type,
	const std::function<bool(InfoTypePair, InfoMethodPair)>& func,
	MethodFlag flag) const
{
	ForEachTypeInfo(type, [flag, &func](InfoTypePair type) {
		for (auto& [method_name, methodInfo] : type.info->methodinfos) {
			if (!enum_contain(flag, methodInfo.methodptr.GetMethodFlag()))
				continue;

			if (!func(type, { method_name, &methodInfo }))
				return false;
		}
		return true;
	});
}

void ReflMngr::ForEachVar(
	ObjectView obj,
	const std::function<bool(InfoTypePair, InfoFieldPair, ObjectView)>& func,
	FieldFlag flag) const
{
	if (!obj.GetPtr())
		flag = enum_within(flag, FieldFlag::Unowned);

	std::set<TypeID> visitedVBs;
	const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		details::ForEachVar(obj.RemoveLValueReference(), [&func](InfoTypePair t, InfoFieldPair f, ObjectView o) {
			return func(t, f, o.AddLValueReference());
			}, flag, visitedVBs);
		break;
	case CVRefMode::Right:
		details::ForEachVar(obj.RemoveRValueReference(), [&func](InfoTypePair t, InfoFieldPair f, ObjectView o) {
			return func(t, f, o.AddRValueReference());
		}, flag, visitedVBs);
		break;
	case CVRefMode::Const:
		details::ForEachVar(obj.RemoveConst(), [&func](InfoTypePair t, InfoFieldPair f, ObjectView o) {
			return func(t, f, o.AddConst());
		}, flag, visitedVBs);
		break;
	case CVRefMode::ConstLeft:
		details::ForEachVar(obj.RemoveConstReference(), [&func](InfoTypePair t, InfoFieldPair f, ObjectView o) {
			return func(t, f, o.AddConstLValueReference());
		}, flag, visitedVBs);
		break;
	case CVRefMode::ConstRight:
		details::ForEachVar(obj.RemoveConstReference(), [&func](InfoTypePair t, InfoFieldPair f, ObjectView o) {
			return func(t, f, o.AddConstRValueReference());
		}, flag, visitedVBs);
		break;
	default:
		details::ForEachVar(obj, func, flag, visitedVBs);
		break;
	}
}

std::vector<InfoTypePair> ReflMngr::GetTypes(Type type) {
	std::vector<InfoTypePair> rst;
	ForEachTypeInfo(type, [&rst](InfoTypePair type) {
		rst.push_back(type);
		return true;
	});
	return rst;
}

std::vector<InfoTypeFieldPair> ReflMngr::GetTypeFields(Type type, FieldFlag flag) {
	std::vector<InfoTypeFieldPair> rst;
	ForEachField(type, [&rst](InfoTypePair type, InfoFieldPair field) {
		rst.emplace_back(InfoTypeFieldPair{ type, field });
		return true;
	}, flag);
	return rst;
}

std::vector<InfoFieldPair> ReflMngr::GetFields(Type type, FieldFlag flag) {
	std::vector<InfoFieldPair> rst;
	ForEachField(type, [&rst](InfoTypePair type, InfoFieldPair field) {
		rst.push_back(field);
		return true;
	}, flag);
	return rst;
}

std::vector<InfoTypeMethodPair> ReflMngr::GetTypeMethods(Type type, MethodFlag flag) {
	std::vector<InfoTypeMethodPair> rst;
	ForEachMethod(type, [&rst](InfoTypePair type, InfoMethodPair field) {
		rst.emplace_back(InfoTypeMethodPair{ type, field });
		return true;
	}, flag);
	return rst;
}

std::vector<InfoMethodPair> ReflMngr::GetMethods(Type type, MethodFlag flag) {
	std::vector<InfoMethodPair> rst;
	ForEachMethod(type, [&rst](InfoTypePair type, InfoMethodPair field) {
		rst.push_back(field);
		return true;
	}, flag);
	return rst;
}

std::vector<std::tuple<InfoTypePair, InfoFieldPair, ObjectView>> ReflMngr::GetTypeFieldVars(ObjectView obj, FieldFlag flag) {
	std::vector<std::tuple<InfoTypePair, InfoFieldPair, ObjectView>> rst;
	ForEachVar(obj, [&rst](InfoTypePair type, InfoFieldPair field, ObjectView var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	}, flag);
	return rst;
}

std::vector<ObjectView> ReflMngr::GetVars(ObjectView obj, FieldFlag flag) {
	std::vector<ObjectView> rst;
	ForEachVar(obj, [&rst](InfoTypePair type, InfoFieldPair field, ObjectView var) {
		rst.push_back(var);
		return true;
	}, flag);
	return rst;
}

InfoTypePair ReflMngr::FindType(Type type, const std::function<bool(InfoTypePair)>& func) const {
	InfoTypePair rst;
	ForEachTypeInfo(type, [&rst, func](InfoTypePair type) {
		if (!func(type))
			return true;

		rst = type;
		return false; // stop
	});
	return rst;
}

InfoFieldPair ReflMngr::FindField(Type type, const std::function<bool(InfoFieldPair)>& func, FieldFlag flag) const {
	InfoFieldPair rst;
	ForEachField(type, [&rst, func](InfoTypePair type, InfoFieldPair field) {
		if (!func(field))
			return true;

		rst = field;
		return false; // stop
	}, flag);
	return rst;
}

InfoMethodPair ReflMngr::FindMethod(Type type, const std::function<bool(InfoMethodPair)>& func, MethodFlag flag) const {
	InfoMethodPair rst;
	ForEachMethod(type, [&rst, func](InfoTypePair type, InfoMethodPair method) {
		if (!func(method))
			return true;

		rst = method;
		return false; // stop
	}, flag);
	return rst;
}

ObjectView ReflMngr::FindVar(ObjectView obj, const std::function<bool(ObjectView)>& func, FieldFlag flag) const {
	ObjectView rst;
	ForEachVar(obj, [&rst, func](InfoTypePair type, InfoFieldPair field, ObjectView obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	}, flag);
	return rst;
}

bool ReflMngr::ContainsBase(Type type, Type base) const {
	auto* info = GetTypeInfo(type);
	if (info->baseinfos.contains(base.RemoveCVRef()))
		return true;

	for (const auto& [basetype, baseinfo] : info->baseinfos) {
		bool found = ContainsBase(basetype, base);
		if (found)
			return true;
	}
	return false;
}

bool ReflMngr::ContainsField(Type type, Name field_name, FieldFlag flag) const {
	const CVRefMode cvref_mode = type.GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		return details::ContainsField(type.RemoveLValueReference(), field_name, flag);
	case CVRefMode::Right:
		return details::ContainsField(type.RemoveRValueReference(), field_name, flag);
	case CVRefMode::Const:
		return details::ContainsField(type.RemoveConst(), field_name, flag);
	case CVRefMode::ConstLeft:
	case CVRefMode::ConstRight:
		return details::ContainsField(type.RemoveCVRef(), field_name, flag);
	default:
		return details::ContainsField(type, field_name, flag);
	}
}

bool ReflMngr::ContainsMethod(Type type, Name method_name, MethodFlag flag) const {
	const CVRefMode cvref_mode = type.GetCVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		return details::ContainsMethod(type.RemoveLValueReference(), method_name, flag);
	case CVRefMode::Right:
		return details::ContainsMethod(type.RemoveRValueReference(), method_name, flag);
	case CVRefMode::Const:
		return details::ContainsMethod(type.RemoveConst(), method_name, flag);
	case CVRefMode::ConstLeft:
	case CVRefMode::ConstRight:
		return details::ContainsMethod(type.RemoveCVRef(), method_name, flag);
	default:
		return details::ContainsMethod(type, method_name, flag);
	}
}
