#include <UDRefl/ReflMngr.h>

#include <set>
#include <array>

using namespace Ubpa::UDRefl;

namespace Ubpa::UDRefl::details {
	static void ForEachTypeID(
		TypeID typeID,
		const std::function<void(TypeID)>& func,
		std::set<TypeID>& visitedVBs)
	{
		func(typeID);

		auto target = ReflMngr::Instance().typeinfos.find(typeID);

		if (target == ReflMngr::Instance().typeinfos.end())
			return;

		const auto& typeinfo = target->second;

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			details::ForEachTypeID(baseID, func, visitedVBs);
		}
	}

	static void ForEachTypeInfo(
		TypeID typeID,
		const std::function<void(Type)>& func,
		std::set<TypeID>& visitedVBs)
	{

		auto target = ReflMngr::Instance().typeinfos.find(typeID);

		if (target == ReflMngr::Instance().typeinfos.end())
			return;

		const auto& typeinfo = target->second;

		func({ typeID, typeinfo });

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			details::ForEachTypeInfo(baseID, func, visitedVBs);
		}
	}

	static void ForEachRWVar(
		ObjectPtr obj,
		const std::function<void(Type, Field, ObjectPtr)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!obj)
			return;

		auto target = ReflMngr::Instance().typeinfos.find(obj.GetID());

		if (target == ReflMngr::Instance().typeinfos.end())
			return;

		auto& typeinfo = target->second;

		for (auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
			if (!fieldInfo.fieldptr.IsConst())
				func({ obj.GetID(), typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.Map(obj));
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			details::ForEachRWVar(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, func, visitedVBs);
		}
	}

	static void ForEachRVar(
		ConstObjectPtr obj,
		const std::function<void(Type, Field, ConstObjectPtr)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!obj)
			return;

		auto target = ReflMngr::Instance().typeinfos.find(obj.GetID());

		if (target == ReflMngr::Instance().typeinfos.end())
			return;

		const auto& typeinfo = target->second;

		for (const auto& [fieldID, fieldInfo] : typeinfo.fieldinfos)
			func({ obj.GetID(), typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.Map(obj));

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			details::ForEachRVar(ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, func, visitedVBs);
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
			{NameRegistry::DirectGetID(NameRegistry::Meta::malloc), std::move(methodinfo_malloc)},
			{NameRegistry::DirectGetID(NameRegistry::Meta::free), std::move(methodinfo_free)},
			{NameRegistry::DirectGetID(NameRegistry::Meta::aligned_malloc), std::move(methodinfo_aligned_malloc)},
			{NameRegistry::DirectGetID(NameRegistry::Meta::aligned_free), std::move(methodinfo_aligned_free)},
		},
	};

	typeinfos.emplace(tregistry.GetID(TypeRegistry::Meta::global), std::move(global));
}

void* ReflMngr::Malloc(uint64_t size) const {
	std::array argsTypeIDs = { TypeRegistry::DirectGetID<uint64_t>() };

	std::uint8_t args_buffer[sizeof(uint64_t)];
	buffer_get<uint64_t>(args_buffer, 0) = size;

	std::uint8_t result_buffer[sizeof(void*)];

	auto result = Invoke(NameRegistry::DirectGetID(NameRegistry::Meta::malloc), argsTypeIDs, args_buffer, result_buffer);

	if (result.success)
		return buffer_get<void*>(result_buffer, 0);
	else
		return nullptr;
}

bool ReflMngr::Free(void* ptr) const {
	std::array argsTypeIDs = { TypeRegistry::DirectGetID<void*>() };

	std::uint8_t args_buffer[sizeof(void*)];
	buffer_get<void*>(args_buffer, 0) = ptr;

	auto result = Invoke(NameRegistry::DirectGetID(NameRegistry::Meta::free), argsTypeIDs, args_buffer);

	return result.success;
}

void* ReflMngr::AlignedMalloc(uint64_t size, uint64_t alignment) const {
	std::array argsTypeIDs = { TypeRegistry::DirectGetID<uint64_t>(),TypeRegistry::DirectGetID<uint64_t>() };

	std::uint8_t args_buffer[2 * sizeof(uint64_t)];
	buffer_get<uint64_t>(args_buffer, 0) = size;
	buffer_get<uint64_t>(args_buffer, sizeof(uint64_t)) = alignment;

	std::uint8_t result_buffer[sizeof(void*)];
	
	auto result = Invoke(NameRegistry::DirectGetID(NameRegistry::Meta::aligned_malloc), argsTypeIDs, args_buffer, result_buffer);

	if (result.success)
		return buffer_get<void*>(result_buffer, 0);
	else
		return nullptr;
}

bool ReflMngr::AlignedFree(void* ptr) const {
	std::array argsTypeIDs = { TypeRegistry::DirectGetID<void*>() };

	std::uint8_t args_buffer[sizeof(void*)];
	buffer_get<void*>(args_buffer, 0) = ptr;

	auto result = Invoke(NameRegistry::DirectGetID(NameRegistry::Meta::aligned_free), argsTypeIDs, args_buffer);

	return result.success;
}

ObjectPtr ReflMngr::New(TypeID typeID, Span<TypeID> argTypeIDs, void* args_buffer) const {
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

SharedObject ReflMngr::MakeShared(TypeID typeID, Span<TypeID> argTypeIDs, void* args_buffer) const {
	ObjectPtr obj = New(typeID, argTypeIDs, args_buffer);
	return { obj, [typeID](void* ptr) {
		bool success = ReflMngr::Instance().Delete({typeID, ptr});
		assert(success);
	}};
}

ObjectPtr ReflMngr::StaticCast_DerivedToBase(ObjectPtr obj, TypeID typeID) const noexcept {
	assert(typeID != static_cast<size_t>(-1));

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
		if (ptr.GetID() != static_cast<size_t>(-1))
			return ptr;
	}

	return nullptr;
}

ObjectPtr ReflMngr::StaticCast_BaseToDerived(ObjectPtr obj, TypeID typeID) const noexcept {
	assert(typeID != static_cast<size_t>(-1));

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
		if (ptr.GetID() != static_cast<size_t>(-1))
			return { baseID, baseinfo.IsVirtual() ? nullptr : baseinfo.StaticCast_BaseToDerived(obj) };
	}
	
	return nullptr;
}

ObjectPtr ReflMngr::DynamicCast_BaseToDerived(ObjectPtr obj, TypeID typeID) const noexcept {
	assert(typeID != static_cast<size_t>(-1));

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
		if (ptr.GetID() != static_cast<size_t>(-1))
			return { baseID, baseinfo.IsPolymorphic() ? baseinfo.DynamicCast_BaseToDerived(obj) : nullptr };
	}

	return nullptr;
}

ObjectPtr ReflMngr::StaticCast(ObjectPtr obj, TypeID typeID) const noexcept {
	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetID() != static_cast<size_t>(-1))
		return ptr_d2b;

	auto ptr_b2d = StaticCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetID() != static_cast<size_t>(-1))
		return ptr_b2d;

	return nullptr;
}

ObjectPtr ReflMngr::DynamicCast(ObjectPtr obj, TypeID typeID) const noexcept {
	auto ptr_b2d = DynamicCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetID() != static_cast<size_t>(-1))
		return ptr_b2d;

	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetID() != static_cast<size_t>(-1))
		return ptr_d2b;

	return nullptr;
}

ObjectPtr ReflMngr::RWVar(TypeID typeID, NameID fieldID) noexcept {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return nullptr;

	auto& typeinfo = target->second;

	auto ptr = typeinfo.RWVar(fieldID);
	if (ptr)
		return ptr;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RWVar(baseID, fieldID);
		if (bptr)
			return bptr;
	}

	return nullptr;
}

ConstObjectPtr ReflMngr::RVar(TypeID typeID, NameID fieldID) const noexcept {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return nullptr;

	auto& typeinfo = target->second;

	auto ptr = typeinfo.RVar(fieldID);
	if (ptr)
		return ptr;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RVar(baseID, fieldID);
		if (bptr)
			return bptr;
	}

	return nullptr;
}

ObjectPtr ReflMngr::RWVar(ObjectPtr obj, NameID fieldID) noexcept {
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return nullptr;

	auto& typeinfo = target->second;

	auto ptr = typeinfo.RWVar(obj, fieldID);
	if (ptr)
		return ptr;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RWVar(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, fieldID);
		if (bptr)
			return bptr;
	}

	return nullptr;
}

ConstObjectPtr ReflMngr::RVar(ConstObjectPtr obj, NameID fieldID) const noexcept {
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	auto ptr = typeinfo.RVar(obj, fieldID);
	if (ptr)
		return ptr;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RVar(ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, fieldID);
		if (bptr)
			return bptr;
	}

	return nullptr;
}

bool ReflMngr::IsStaticInvocable(
	TypeID typeID,
	NameID methodID,
	Span<TypeID> argTypeIDs) const noexcept
{
	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return false;

	const auto& typeinfo = typetarget->second;

	if (typeinfo.IsStaticInvocable(methodID, argTypeIDs))
		return true;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		if (IsStaticInvocable(baseID, methodID, argTypeIDs))
			return true;
	}
	return false;
}

bool ReflMngr::IsConstInvocable(
	TypeID typeID,
	NameID methodID,
	Span<TypeID> argTypeIDs) const noexcept
{
	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return false;

	const auto& typeinfo = typetarget->second;

	if (typeinfo.IsConstInvocable(methodID, argTypeIDs))
		return true;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		if (IsConstInvocable(baseID, methodID, argTypeIDs))
			return true;
	}

	return false;
}

bool ReflMngr::IsInvocable(
	TypeID typeID,
	NameID methodID,
	Span<TypeID> argTypeIDs) const noexcept
{
	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return false;

	const auto& typeinfo = typetarget->second;

	if (typeinfo.IsInvocable(methodID, argTypeIDs))
		return true;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		if (IsInvocable(baseID, methodID, argTypeIDs))
			return true;
	}

	return false;
}

InvokeResult ReflMngr::Invoke(
	TypeID typeID,
	NameID methodID,
	Span<TypeID> argTypeIDs,
	void* args_buffer,
	void* result_buffer) const
{
	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto rst = typeinfo.Invoke(methodID, argTypeIDs, args_buffer, result_buffer);

	if (rst.success)
		return rst;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = Invoke(baseID, methodID, argTypeIDs, args_buffer, result_buffer);
		if (rst.success)
			return rst;
	}

	return {};
}

InvokeResult ReflMngr::Invoke(
	ConstObjectPtr obj,
	NameID methodID,
	Span<TypeID> argTypeIDs,
	void* args_buffer,
	void* result_buffer) const
{
	auto typetarget = typeinfos.find(obj.GetID());

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto rst = typeinfo.Invoke(obj, methodID, argTypeIDs, args_buffer, result_buffer);

	if (rst.success)
		return rst;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = Invoke(
			ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) },
			methodID, argTypeIDs, args_buffer, result_buffer
		);
		if (rst.success)
			return rst;
	}

	return {};
}

InvokeResult ReflMngr::Invoke(
	ObjectPtr obj,
	NameID methodID,
	Span<TypeID> argTypeIDs,
	void* args_buffer,
	void* result_buffer) const
{
	auto typetarget = typeinfos.find(obj.GetID());

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto rst = typeinfo.Invoke(obj, methodID, argTypeIDs, args_buffer, result_buffer);

	if (rst.success)
		return rst;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = Invoke(
			ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) },
			methodID, argTypeIDs, args_buffer, result_buffer
		);
		if (rst.success)
			return rst;
	}

	return {};
}

bool ReflMngr::IsConstructible(TypeID typeID, Span<TypeID> argTypeIDs) const noexcept {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	return typeinfo.IsInvocable(NameRegistry::DirectGetID(NameRegistry::Meta::ctor), argTypeIDs);
}

bool ReflMngr::IsDestructible(TypeID typeID) const noexcept {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	return typeinfo.IsConstInvocable(NameRegistry::DirectGetID(NameRegistry::Meta::dtor), {});
}

bool ReflMngr::Construct(ObjectPtr obj, Span<TypeID> argTypeIDs, void* args_buffer) const {
	assert(obj);
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	auto rst = typeinfo.Invoke(obj, NameRegistry::DirectGetID(NameRegistry::Meta::ctor), argTypeIDs, args_buffer, nullptr);
	return rst.success;
}

bool ReflMngr::Destruct(ConstObjectPtr obj) const {
	assert(obj);
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	auto rst = typeinfo.Invoke(obj, NameRegistry::DirectGetID(NameRegistry::Meta::dtor), {}, nullptr, nullptr);
	return rst.success;
}

void ReflMngr::ForEachTypeID(TypeID typeID, const std::function<void(TypeID)>& func) const {
	std::set<TypeID> visitedVBs;
	details::ForEachTypeID(typeID, func, visitedVBs);
}

void ReflMngr::ForEachType(TypeID typeID, const std::function<void(Type)>& func) const {
	std::set<TypeID> visitedVBs;
	details::ForEachTypeInfo(typeID, func, visitedVBs);
}

void ReflMngr::ForEachField(
	TypeID typeID,
	const std::function<void(Type, Field)>& func) const
{
	ForEachType(typeID, [&func](Type type) {
		for (const auto& [fieldID, fieldInfo] : type.info.fieldinfos)
			func(type, { fieldID, fieldInfo });
	});
}

void ReflMngr::ForEachMethod(
	TypeID typeID,
	const std::function<void(Type, Method)>& func) const
{
	ForEachType(typeID, [&func](Type type) {
		for (const auto& [methodID, methodInfo] : type.info.methodinfos)
			func(type, { methodID, methodInfo });
	});
}

void ReflMngr::ForEachRWVar(
	ObjectPtr obj,
	const std::function<void(Type, Field, ObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	details::ForEachRWVar(obj, func, visitedVBs);
}

void ReflMngr::ForEachRVar(
	ConstObjectPtr obj,
	const std::function<void(Type, Field, ConstObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	details::ForEachRVar(obj, func, visitedVBs);
}
