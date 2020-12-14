#include <UDRefl/ReflMngr.h>

#include <set>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

namespace Ubpa::UDRefl::details {
	static bool IsStaticInvokable(const TypeInfo& typeinfo, StrID methodID, Span<const TypeID> argTypeIDs) {
		auto mtarget = typeinfo.methodinfos.find(methodID);
		size_t num = typeinfo.methodinfos.count(methodID);
		for (size_t i = 0; i < num; ++i, ++mtarget) {
			if (mtarget->second.methodptr.IsStatic()
				&& mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
				return true;
		}
		return false;
	}

	static bool ForEachTypeID(
		TypeID typeID,
		const std::function<bool(TypeID)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!func(typeID))
			return false;

		auto target = ReflMngr::Instance().typeinfos.find(typeID);

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		const auto& typeinfo = target->second;

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachTypeID(baseID, func, visitedVBs))
				return false;
		}
		return true;
	}

	static bool ForEachTypeInfo(
		TypeID typeID,
		const std::function<bool(TypeRef)>& func,
		std::set<TypeID>& visitedVBs)
	{
		auto target = ReflMngr::Instance().typeinfos.find(typeID);

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		if (!func({ typeID, typeinfo }))
			return false;

		for (auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachTypeInfo(baseID, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachRWVar(
		ObjectPtr obj,
		const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!obj)
			return true;

		auto target = ReflMngr::Instance().typeinfos.find(obj.GetID());

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
			if (!fieldInfo.fieldptr.IsConst()) {
				if (!func({ obj.GetID(), typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.Map(obj)))
					return false;
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachRWVar(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachRVar(
		ConstObjectPtr obj,
		const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!obj)
			return true;

		auto target = ReflMngr::Instance().typeinfos.find(obj.GetID());

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
			if (!func({ obj.GetID(), typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.Map(obj)))
				return false;
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachRVar(ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachRWVar(
		TypeID typeID,
		const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!typeID)
			return true;

		auto target = ReflMngr::Instance().typeinfos.find(typeID);

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
			if (fieldInfo.fieldptr.IsObject() && !fieldInfo.fieldptr.IsConst()) {
				if (!func({ typeID, typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.Map()))
					return false;
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachRWVar(baseID, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachRVar(
		TypeID typeID,
		const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!typeID)
			return true;

		auto target = ReflMngr::Instance().typeinfos.find(typeID);

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
			if (fieldInfo.fieldptr.IsObject()) {
				if (!func({ typeID, typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.Map()))
					return false;
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachRVar(baseID, func, visitedVBs))
				return false;
		}

		return true;
	}

	DeleteFunc GenerateDeleteFunc(Destructor&& dtor, MemoryResourceType type, size_t size, size_t alignment) {
		assert(size > 0);
		if (dtor && type != MemoryResourceType::MONO) {
			return [d = std::move(dtor), type, size, alignment](void* ptr) {
				d(ptr);
				ReflMngr::Instance().MDeallocate(type, ptr, size, alignment);
			};
		}
		else if (dtor && type == MemoryResourceType::MONO) {
			return[d = std::move(dtor)] (void* ptr) {
				d(ptr);
			};
		}
		else if (!dtor && type != MemoryResourceType::MONO) {
			return [type, size, alignment] (void* ptr) {
				ReflMngr::Instance().MDeallocate(type, ptr, size, alignment);
			};
		}
		else // !dtor && memory_rsrc_type == MemoryResourceType::MONO
		{
			return [](void* ptr) {
				assert(ptr);
			};
		}
	}
}

ReflMngr::ReflMngr() {
	MethodInfo methodinfo_malloc{ GenerateMethodPtr<&malloc>() };
	MethodInfo methodinfo_free{ GenerateMethodPtr<&free>() };

#if defined(WIN32) || defined(_WINDOWS)
	#define UBPA_UDREFL_ALIGN_MALLOC _aligned_malloc
#else
	#define UBPA_UDREFL_ALIGN_MALLOC aligned_alloc
#endif // defined(WIN32) || defined(_WINDOWS)
	MethodInfo methodinfo_aligned_malloc{ GenerateMethodPtr<&UBPA_UDREFL_ALIGN_MALLOC>() };
#undef UBPA_UDREFL_ALIGN_MALLOC

#if defined(WIN32) || defined(_WINDOWS)
#define UBPA_UDREFL_ALIGN_FREE _aligned_free
#else
#define UBPA_UDREFL_ALIGN_FREE free
#endif // defined(WIN32) || defined(_WINDOWS)
	MethodInfo methodinfo_aligned_free{ GenerateMethodPtr<&UBPA_UDREFL_ALIGN_FREE>() };
#undef UBPA_UDREFL_ALIGN_FREE

	TypeInfo global{
		0,
		0,
		{}, // fieldinfos
		{ // methodinfos
			{StrIDRegistry::MetaID::malloc, std::move(methodinfo_malloc)},
			{StrIDRegistry::MetaID::free, std::move(methodinfo_free)},
			{StrIDRegistry::MetaID::aligned_malloc, std::move(methodinfo_aligned_malloc)},
			{StrIDRegistry::MetaID::aligned_free, std::move(methodinfo_aligned_free)},
		},
	};

	typeinfos.emplace(tregistry.Register(TypeIDRegistry::Meta::global), std::move(global));
}

void ReflMngr::Clear() noexcept {
	// field attrs
	for (auto& [typeID, typeinfo] : typeinfos) {
		for (auto& [fieldID, fieldinfo] : typeinfo.fieldinfos)
			fieldinfo.attrs.clear();
	}

	// type attrs
	for (auto& [ID, typeinfo] : typeinfos)
		typeinfo.attrs.clear();

	// type dynamic field
	for (auto& [typeID, typeinfo] : typeinfos) {
		auto iter = typeinfo.fieldinfos.begin();
		while (iter != typeinfo.fieldinfos.end()) {
			auto cur = iter;
			++iter;

			if (cur->second.fieldptr.IsDynamicShared())
				typeinfo.fieldinfos.erase(cur);
		}
	}

	typeinfos.clear();
}

ReflMngr::~ReflMngr() {
	Clear();
}

TypeID ReflMngr::RegisterType(std::string_view name, size_t size, size_t alignment) {
	TypeID ID = tregistry.Register(name);
	{
		auto target = typeinfos.find(ID);
		if (target == typeinfos.end())
			target = typeinfos.emplace_hint(target, ID, TypeInfo{});
		target->second.size = size;
		target->second.alignment = alignment;
	}
	return ID;
}

StrID ReflMngr::AddField(TypeID typeID, std::string_view name, FieldInfo fieldinfo) {
	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return {};
	auto& typeinfo = ttarget->second;
	StrID fieldID = nregistry.Register(name);
	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end())
		return {};
	typeinfo.fieldinfos.emplace_hint(ftarget, fieldID, std::move(fieldinfo));
	return fieldID;
}

StrID ReflMngr::AddMethod(TypeID typeID, std::string_view name, MethodInfo methodinfo) {
	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return {};
	auto& typeinfo = ttarget->second;
	StrID methodID = nregistry.Register(name);
	size_t mcnt = typeinfo.methodinfos.count(methodID);
	auto miter = typeinfo.methodinfos.find(methodID);
	bool flag = true;
	for (size_t i = 0; i < mcnt; ++i, ++miter) {
		if (!miter->second.methodptr.IsDistinguishableWith(methodinfo.methodptr)) {
			flag = false;
			break;
		}
	}
	if (!flag)
		return {};
	typeinfo.methodinfos.emplace(methodID, std::move(methodinfo));
	return methodID;
}

bool ReflMngr::AddBase(TypeID derivedID, TypeID baseID, BaseInfo baseinfo) {
	auto ttarget = typeinfos.find(derivedID);
	if (ttarget == typeinfos.end())
		return false;
	auto& typeinfo = ttarget->second;
	auto btarget = typeinfo.baseinfos.find(baseID);
	if (btarget != typeinfo.baseinfos.end())
		return false;
	typeinfo.baseinfos.emplace_hint(btarget, baseID, std::move(baseinfo));
	return true;
}

void* ReflMngr::Malloc(size_t size) const {
	return Invoke<void*>(GlobalID, StrIDRegistry::MetaID::malloc, size);
}

bool ReflMngr::Free(void* ptr) const {
	return InvokeArgs(GlobalID, StrIDRegistry::MetaID::free, nullptr, ptr);
}

void* ReflMngr::AlignedMalloc(size_t size, size_t alignment) const {
	return Invoke<void*>(GlobalID, StrIDRegistry::MetaID::aligned_malloc, size, alignment);
}

bool ReflMngr::AlignedFree(void* ptr) const {
	return InvokeArgs(GlobalID, StrIDRegistry::MetaID::aligned_free, nullptr, ptr);
}

ObjectPtr ReflMngr::New(TypeID typeID, Span<const TypeID> argTypeIDs, void* args_buffer) const {
	if (!IsConstructible(typeID, argTypeIDs))
		return nullptr;

	const auto& typeinfo = typeinfos.at(typeID);

	void* buffer = typeinfo.alignment <= std::alignment_of_v<std::max_align_t> ?
		Malloc(typeinfo.size)
		: AlignedMalloc(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectPtr obj{ typeID, buffer };
	bool success = Construct(obj, argTypeIDs, args_buffer);
	
	if (success)
		return obj;
	else
		return nullptr;
}

bool ReflMngr::Delete(ConstObjectPtr obj) const {
	bool dtor_success = Destruct(obj);
	if (!dtor_success)
		return false;

	const auto& typeinfo = typeinfos.at(obj.GetID());

	bool free_success;
	if (typeinfo.alignment <= std::alignment_of_v<std::max_align_t>)
		free_success = Free(const_cast<void*>(obj.GetPtr()));
	else
		free_success = AlignedFree(const_cast<void*>(obj.GetPtr()));

	return free_success;
}

SharedObject ReflMngr::MakeShared(TypeID typeID, Span<const TypeID> argTypeIDs, void* args_buffer) const {
	ObjectPtr obj = New(typeID, argTypeIDs, args_buffer);
	return { obj, [typeID](void* ptr) {
		bool success = ReflMngr::Instance().Delete({typeID, ptr});
		assert(success);
	}};
}

ObjectPtr ReflMngr::StaticCast_DerivedToBase(ObjectPtr obj, TypeID typeID) const noexcept {
	assert(typeID);

	if (obj.GetID() == typeID)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { typeID, nullptr };

	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto ptr = StaticCast_DerivedToBase(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, typeID);
		if (ptr.GetID())
			return ptr;
	}

	return nullptr;
}

ObjectPtr ReflMngr::StaticCast_BaseToDerived(ObjectPtr obj, TypeID typeID) const noexcept {
	assert(typeID);

	if (obj.GetID() == typeID)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { typeID, nullptr };

	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto ptr = StaticCast_BaseToDerived(obj, baseID);
		if (ptr.GetID())
			return { baseID, baseinfo.IsVirtual() ? nullptr : baseinfo.StaticCast_BaseToDerived(obj) };
	}
	
	return nullptr;
}

ObjectPtr ReflMngr::DynamicCast_BaseToDerived(ObjectPtr obj, TypeID typeID) const noexcept {
	assert(typeID);

	if (obj.GetID() == typeID)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { typeID, nullptr };

	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto ptr = DynamicCast_BaseToDerived(ObjectPtr{ baseID, baseinfo.DynamicCast_BaseToDerived(obj) }, typeID);
		if (ptr.GetID())
			return { baseID, baseinfo.IsPolymorphic() ? baseinfo.DynamicCast_BaseToDerived(obj) : nullptr };
	}

	return nullptr;
}

ObjectPtr ReflMngr::StaticCast(ObjectPtr obj, TypeID typeID) const noexcept {
	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetID())
		return ptr_d2b;

	auto ptr_b2d = StaticCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetID())
		return ptr_b2d;

	return nullptr;
}

ObjectPtr ReflMngr::DynamicCast(ObjectPtr obj, TypeID typeID) const noexcept {
	auto ptr_b2d = DynamicCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetID())
		return ptr_b2d;

	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetID())
		return ptr_d2b;

	return nullptr;
}

ObjectPtr ReflMngr::RWVar(TypeID typeID, StrID fieldID) noexcept {
	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;

	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end() && ftarget->second.fieldptr.IsObject())
		return ftarget->second.fieldptr.Map();

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RWVar(baseID, fieldID);
		if (bptr)
			return bptr;
	}

	return nullptr;
}

ConstObjectPtr ReflMngr::RVar(TypeID typeID, StrID fieldID) const noexcept {
	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;


	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end() && ftarget->second.fieldptr.IsObject())
		return ftarget->second.fieldptr.Map();

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RVar(baseID, fieldID);
		if (bptr)
			return bptr;
	}

	return nullptr;
}

ObjectPtr ReflMngr::RWVar(ObjectPtr obj, StrID fieldID) noexcept {
	auto ttarget = typeinfos.find(obj.GetID());
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;


	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end() && ftarget->second.fieldptr.IsVariable())
		return ftarget->second.fieldptr.Map(obj);

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RWVar(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, fieldID);
		if (bptr)
			return bptr;
	}

	return nullptr;
}

ConstObjectPtr ReflMngr::RVar(ConstObjectPtr obj, StrID fieldID) const noexcept {
	auto ttarget = typeinfos.find(obj.GetID());
	if (ttarget == typeinfos.end())
		return nullptr;

	const auto& typeinfo = ttarget->second;

	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end())
		return ftarget->second.fieldptr.Map(obj);

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RVar(ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, fieldID);
		if (bptr)
			return bptr;
	}

	return nullptr;
}

ObjectPtr ReflMngr::RWVar(ObjectPtr obj, TypeID baseID, StrID fieldID) noexcept {
	auto base = StaticCast_DerivedToBase(obj, baseID);
	if (!base)
		return nullptr;
	return RWVar(base, fieldID);
}

ConstObjectPtr ReflMngr::RVar(ConstObjectPtr obj, TypeID baseID, StrID fieldID) const noexcept {
	auto base = StaticCast_DerivedToBase(obj, baseID);
	if (!base)
		return nullptr;
	return RVar(base, fieldID);
}

bool ReflMngr::IsStaticInvocable(
	TypeID typeID,
	StrID methodID,
	Span<const TypeID> argTypeIDs) const noexcept
{
	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return false;

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.IsStatic()
			&& mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		if (IsStaticInvocable(baseID, methodID, argTypeIDs))
			return true;
	}

	return false;
}

bool ReflMngr::IsConstInvocable(
	TypeID typeID,
	StrID methodID,
	Span<const TypeID> argTypeIDs) const noexcept
{
	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return false;

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (!mtarget->second.methodptr.IsMemberVariable()
			&& mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		if (IsConstInvocable(baseID, methodID, argTypeIDs))
			return true;
	}

	return false;
}

bool ReflMngr::IsInvocable(
	TypeID typeID,
	StrID methodID,
	Span<const TypeID> argTypeIDs) const noexcept
{
	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return false;

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		if (IsInvocable(baseID, methodID, argTypeIDs))
			return true;
	}

	return false;
}

InvokeResult ReflMngr::Invoke(
	TypeID typeID,
	StrID methodID,
	void* result_buffer,
	Span<const TypeID> argTypeIDs,
	void* args_buffer) const
{
	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.IsStatic()
			&& mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
		{
			return {
				true,
				mtarget->second.methodptr.GetResultDesc().typeID,
				mtarget->second.methodptr.Invoke_Static(result_buffer, args_buffer)
			};
		}
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = Invoke(baseID, methodID, result_buffer, argTypeIDs, args_buffer);
		if (rst.success)
			return rst;
	}

	return {};
}

InvokeResult ReflMngr::Invoke(
	ConstObjectPtr obj,
	StrID methodID,
	void* result_buffer,
	Span<const TypeID> argTypeIDs,
	void* args_buffer) const
{
	auto typetarget = typeinfos.find(obj.GetID());

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (!mtarget->second.methodptr.IsMemberVariable()
			&& mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
		{
			return {
				true,
				mtarget->second.methodptr.GetResultDesc().typeID,
				mtarget->second.methodptr.Invoke(obj, result_buffer, args_buffer)
			};
		}
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = Invoke(
			ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) },
			methodID, result_buffer, argTypeIDs, args_buffer
		);
		if (rst.success)
			return rst;
	}

	return {};
}

InvokeResult ReflMngr::Invoke(
	ObjectPtr obj,
	StrID methodID,
	void* result_buffer,
	Span<const TypeID> argTypeIDs,
	void* args_buffer) const
{
	auto typetarget = typeinfos.find(obj.GetID());

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);

	{ // 1. object variable and static
		auto iter = mtarget;
		for (size_t i = 0; i < num; ++i, ++iter) {
			if (!iter->second.methodptr.IsMemberConst()
				&& iter->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			{
				return {
					true,
					iter->second.methodptr.GetResultDesc().typeID,
					iter->second.methodptr.Invoke(obj, result_buffer, args_buffer)
				};
			}
		}
	}

	{ // 2. object const
		auto iter = mtarget;
		for (size_t i = 0; i < num; ++i, ++iter) {
			if (iter->second.methodptr.IsMemberConst()
				&& iter->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			{
				return {
					true,
					iter->second.methodptr.GetResultDesc().typeID,
					iter->second.methodptr.Invoke(obj, result_buffer, args_buffer)
				};
			}
		}
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = Invoke(
			ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) },
			methodID, result_buffer, argTypeIDs, args_buffer
		);
		if (rst.success)
			return rst;
	}

	return {};
}

void ReflMngr::ReleaseMono() {
	mono_rsrc.release();
}

void* ReflMngr::MAllocate(MemoryResourceType type, size_t size, size_t alignment) {
	switch (type)
	{
	case Ubpa::UDRefl::MemoryResourceType::MONO:
		return mono_rsrc.allocate(size, alignment);
	case Ubpa::UDRefl::MemoryResourceType::SYNC:
		return sync_rsrc.allocate(size, alignment);
	case Ubpa::UDRefl::MemoryResourceType::UNSYNC:
		return unsync_rsrc.allocate(size, alignment);
	default:
		assert(false);
		return nullptr;
	}
}

void ReflMngr::MDeallocate(MemoryResourceType type, void* ptr, size_t size, size_t alignment) {
	assert(ptr);
	switch (type)
	{
	case Ubpa::UDRefl::MemoryResourceType::MONO:
		return mono_rsrc.deallocate(ptr, size, alignment);
	case Ubpa::UDRefl::MemoryResourceType::SYNC:
		return sync_rsrc.deallocate(ptr, size, alignment);
	case Ubpa::UDRefl::MemoryResourceType::UNSYNC:
		return unsync_rsrc.deallocate(ptr, size, alignment);
	default:
		break;
	}
}

SharedObject ReflMngr::MInvoke(
	TypeID typeID,
	StrID methodID,
	Span<const TypeID> argTypeIDs,
	void* args_buffer,
	MemoryResourceType memory_rsrc_type)
{
	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.IsStatic()
			&& mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
		{
			const auto& methodptr = mtarget->second.methodptr;
			const auto& rst_desc = methodptr.GetResultDesc();
			if (rst_desc.IsVoid()) {
				auto dtor = mtarget->second.methodptr.Invoke_Static(nullptr, args_buffer);
				return {
					{rst_desc.typeID, nullptr},
					[](void* ptr) {
						assert(ptr);
					}
				};
			}
			else {
				void* result_buffer = MAllocate(memory_rsrc_type, rst_desc.size, rst_desc.alignment);
				auto dtor = mtarget->second.methodptr.Invoke_Static(result_buffer, args_buffer);
				return {
					{rst_desc.typeID, result_buffer},
					details::GenerateDeleteFunc(std::move(dtor), memory_rsrc_type, rst_desc.size, rst_desc.alignment)
				};
			}
		}
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = MInvoke(baseID, methodID, argTypeIDs, args_buffer, memory_rsrc_type);
		if (rst)
			return rst;
	}

	return nullptr;
}

SharedObject ReflMngr::MInvoke(
	ConstObjectPtr obj,
	StrID methodID,
	Span<const TypeID> argTypeIDs,
	void* args_buffer,
	MemoryResourceType memory_rsrc_type)
{
	auto typetarget = typeinfos.find(obj.GetID());

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (!mtarget->second.methodptr.IsMemberVariable()
			&& mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
		{
			const auto& methodptr = mtarget->second.methodptr;
			const auto& rst_desc = methodptr.GetResultDesc();
			if (rst_desc.IsVoid()) {
				auto dtor = mtarget->second.methodptr.Invoke(obj, nullptr, args_buffer);
				return {
					{rst_desc.typeID, nullptr},
					[](void* ptr) { assert(!ptr); }
				};
			}
			else {
				void* result_buffer = MAllocate(memory_rsrc_type, rst_desc.size, rst_desc.alignment);
				auto dtor = mtarget->second.methodptr.Invoke(obj, result_buffer, args_buffer);
				return {
					{rst_desc.typeID, result_buffer},
					details::GenerateDeleteFunc(std::move(dtor), memory_rsrc_type, rst_desc.size, rst_desc.alignment)
				};
			}
		}
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = MInvoke(
			ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) },
			methodID, argTypeIDs, args_buffer, memory_rsrc_type
		);
		if (rst)
			return rst;
	}

	return nullptr;
}

SharedObject ReflMngr::MInvoke(
	ObjectPtr obj,
	StrID methodID,
	Span<const TypeID> argTypeIDs,
	void* args_buffer,
	MemoryResourceType memory_rsrc_type)
{
	auto typetarget = typeinfos.find(obj.GetID());

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);

	{ // 1. object variable and static
		auto iter = mtarget;
		for (size_t i = 0; i < num; ++i, ++iter) {
			if (!iter->second.methodptr.IsMemberConst()
				&& iter->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			{
				const auto& methodptr = iter->second.methodptr;
				const auto& rst_desc = methodptr.GetResultDesc();

				if (rst_desc.IsVoid()) {
					auto dtor = iter->second.methodptr.Invoke(obj, nullptr, args_buffer);
					return {
						{rst_desc.typeID, nullptr},
						[](void* ptr) { assert(!ptr); }
					};
				}
				else {
					void* result_buffer = MAllocate(memory_rsrc_type, rst_desc.size, rst_desc.alignment);
					auto dtor = iter->second.methodptr.Invoke(obj, result_buffer, args_buffer);
					return {
						{rst_desc.typeID, result_buffer},
						details::GenerateDeleteFunc(std::move(dtor), memory_rsrc_type, rst_desc.size, rst_desc.alignment)
					};
				}
			}
		}
	}

	{ // 2. object const
		auto iter = mtarget;
		for (size_t i = 0; i < num; ++i, ++iter) {
			if (iter->second.methodptr.IsMemberConst()
				&& iter->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			{
				const auto& methodptr = mtarget->second.methodptr;
				const auto& rst_desc = methodptr.GetResultDesc();
				if (rst_desc.IsVoid()) {
					auto dtor = iter->second.methodptr.Invoke(ConstObjectPtr{ obj }, nullptr, args_buffer);
					return {
						{rst_desc.typeID, nullptr},
						[](void* ptr) { assert(!ptr); }
					};
				}
				else {
					void* result_buffer = MAllocate(memory_rsrc_type, rst_desc.size, rst_desc.alignment);
					auto dtor = iter->second.methodptr.Invoke(ConstObjectPtr{ obj }, result_buffer, args_buffer);
					return {
						{rst_desc.typeID, result_buffer},
						details::GenerateDeleteFunc(std::move(dtor), memory_rsrc_type, rst_desc.size, rst_desc.alignment)
					};
				}
			}
		}
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = MInvoke(
			ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) },
			methodID, argTypeIDs, args_buffer, memory_rsrc_type
		);
		if (rst)
			return rst;
	}

	return nullptr;
}

bool ReflMngr::IsConstructible(TypeID typeID, Span<const TypeID> argTypeIDs) const noexcept {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto ctorID = StrIDRegistry::MetaID::ctor;
	auto mtarget = typeinfo.methodinfos.find(ctorID);
	size_t num = typeinfo.methodinfos.count(ctorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}
	return false;
}

bool ReflMngr::IsDestructible(TypeID typeID) const noexcept {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto dtorID = StrIDRegistry::MetaID::dtor;

	auto mtarget = typeinfo.methodinfos.find(dtorID);
	size_t num = typeinfo.methodinfos.count(dtorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (!mtarget->second.methodptr.IsMemberVariable()
			&& mtarget->second.methodptr.GetParamList().IsConpatibleWith({}))
			return true;
	}
	return false;
}

bool ReflMngr::Construct(ObjectPtr obj, Span<const TypeID> argTypeIDs, void* args_buffer) const {
	assert(obj);
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto ctorID = StrIDRegistry::MetaID::ctor;
	auto mtarget = typeinfo.methodinfos.find(ctorID);
	size_t num = typeinfo.methodinfos.count(ctorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.IsMemberVariable()
			&& mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
		{
			mtarget->second.methodptr.Invoke(obj, nullptr, args_buffer);
			return true;
		}
	}
	return false;
}

bool ReflMngr::Destruct(ConstObjectPtr obj) const {
	assert(obj);
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto dtorID = StrIDRegistry::MetaID::dtor;
	auto mtarget = typeinfo.methodinfos.find(dtorID);
	size_t num = typeinfo.methodinfos.count(dtorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.IsMemberConst()
			&& mtarget->second.methodptr.GetParamList().IsConpatibleWith({}))
		{
			mtarget->second.methodptr.Invoke(obj, nullptr, {});
			return true;
		}
	}
	return false;
}

void ReflMngr::ForEachTypeID(TypeID typeID, const std::function<bool(TypeID)>& func) const {
	std::set<TypeID> visitedVBs;
	details::ForEachTypeID(typeID, func, visitedVBs);
}

void ReflMngr::ForEachType(TypeID typeID, const std::function<bool(TypeRef)>& func) const {
	std::set<TypeID> visitedVBs;
	details::ForEachTypeInfo(typeID, func, visitedVBs);
}

void ReflMngr::ForEachField(
	TypeID typeID,
	const std::function<bool(TypeRef, FieldRef)>& func) const
{
	ForEachType(typeID, [&func](TypeRef type) {
		for (auto& [fieldID, fieldInfo] : type.info.fieldinfos) {
			if (func(type, { fieldID, fieldInfo }))
				return false;
		}
		return true;
	});
}

void ReflMngr::ForEachMethod(
	TypeID typeID,
	const std::function<bool(TypeRef, MethodRef)>& func) const
{
	ForEachType(typeID, [&func](TypeRef type) {
		for (auto& [methodID, methodInfo] : type.info.methodinfos) {
			if (!func(type, { methodID, methodInfo }))
				return false;
		}
		return true;
	});
}

void ReflMngr::ForEachRWVar(
	TypeID typeID,
	const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	details::ForEachRWVar(typeID, func, visitedVBs);
}

void ReflMngr::ForEachRVar(
	TypeID typeID,
	const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	details::ForEachRVar(typeID, func, visitedVBs);
}

void ReflMngr::ForEachRWVar(
	ObjectPtr obj,
	const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	details::ForEachRWVar(obj, func, visitedVBs);
}

void ReflMngr::ForEachRVar(
	ConstObjectPtr obj,
	const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	details::ForEachRVar(obj, func, visitedVBs);
}

std::pmr::vector<TypeID> ReflMngr::GetTypeIDs(TypeID typeID) {
	std::pmr::vector<TypeID> rst{ std::pmr::polymorphic_allocator<TypeID>{&sync_rsrc} };
	ForEachTypeID(typeID, [&rst](TypeID typeID) {
		rst.push_back(typeID);
		return true;
	});
	return rst;
}

std::pmr::vector<TypeRef> ReflMngr::GetTypes(TypeID typeID) {
	std::pmr::vector<TypeRef> rst{ std::pmr::polymorphic_allocator<TypeRef>{&sync_rsrc} };
	ForEachType(typeID, [&rst](TypeRef type) {
		rst.push_back(type);
		return true;
	});
	return rst;
}

std::pmr::vector<TypeFieldRef> ReflMngr::GetTypeFields(TypeID typeID) {
	std::pmr::vector<TypeFieldRef> rst{ std::pmr::polymorphic_allocator<TypeFieldRef>{&sync_rsrc} };
	ForEachField(typeID, [&rst](TypeRef type, FieldRef field) {
		rst.emplace_back(TypeFieldRef{ type, field });
		return true;
	});
	return rst;
}

std::pmr::vector<FieldRef> ReflMngr::GetFields(TypeID typeID) {
	std::pmr::vector<FieldRef> rst{ std::pmr::polymorphic_allocator<FieldRef>{&sync_rsrc} };
	ForEachField(typeID, [&rst](TypeRef type, FieldRef field) {
		rst.push_back(field);
		return true;
	});
	return rst;
}

std::pmr::vector<TypeMethodRef> ReflMngr::GetTypeMethods(TypeID typeID) {
	std::pmr::vector<TypeMethodRef> rst{ std::pmr::polymorphic_allocator<TypeMethodRef>{&sync_rsrc} };
	ForEachMethod(typeID, [&rst](TypeRef type, MethodRef field) {
		rst.emplace_back(TypeMethodRef{ type, field });
		return true;
	});
	return rst;
}

std::pmr::vector<MethodRef> ReflMngr::GetMethods(TypeID typeID) {
	std::pmr::vector<MethodRef> rst{ std::pmr::polymorphic_allocator<MethodRef>{&sync_rsrc} };
	ForEachMethod(typeID, [&rst](TypeRef type, MethodRef field) {
		rst.push_back(field);
		return true;
	});
	return rst;
}

std::pmr::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> ReflMngr::GetTypeFieldRWVars(TypeID typeID) {
	std::pmr::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> rst
		{ std::pmr::polymorphic_allocator<std::tuple<TypeRef, FieldRef, ObjectPtr>>{&sync_rsrc} };
	ForEachRWVar(typeID, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::pmr::vector<ObjectPtr> ReflMngr::GetRWVars(TypeID typeID) {
	std::pmr::vector<ObjectPtr> rst{ std::pmr::polymorphic_allocator<ObjectPtr>{&sync_rsrc} };
	ForEachRWVar(typeID, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}

std::pmr::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> ReflMngr::GetTypeFieldRVars(TypeID typeID) {
	std::pmr::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> rst
		{ std::pmr::polymorphic_allocator<std::tuple<TypeRef, FieldRef, ConstObjectPtr>>{&sync_rsrc} };
	ForEachRVar(typeID, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::pmr::vector<ConstObjectPtr> ReflMngr::GetRVars(TypeID typeID) {
	std::pmr::vector<ConstObjectPtr> rst{ std::pmr::polymorphic_allocator<ConstObjectPtr>{&sync_rsrc} };
	ForEachRVar(typeID, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}

std::pmr::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> ReflMngr::GetTypeFieldRWVars(ObjectPtr obj) {
	std::pmr::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> rst
		{ std::pmr::polymorphic_allocator<std::tuple<TypeRef, FieldRef, ObjectPtr>>{&sync_rsrc} };
	ForEachRWVar(obj, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::pmr::vector<ObjectPtr> ReflMngr::GetRWVars(ObjectPtr obj) {
	std::pmr::vector<ObjectPtr> rst{ std::pmr::polymorphic_allocator<ObjectPtr>{&sync_rsrc} };
	ForEachRWVar(obj, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}


std::pmr::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> ReflMngr::GetTypeFieldRVars(ConstObjectPtr obj) {
	std::pmr::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> rst	
		{ std::pmr::polymorphic_allocator<std::tuple<TypeRef, FieldRef, ConstObjectPtr>>{&sync_rsrc} };
	ForEachRVar(obj, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::pmr::vector<ConstObjectPtr> ReflMngr::GetRVars(ConstObjectPtr obj) {
	std::pmr::vector<ConstObjectPtr> rst{ std::pmr::polymorphic_allocator<ConstObjectPtr>{&sync_rsrc} };
	ForEachRVar(obj, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}
