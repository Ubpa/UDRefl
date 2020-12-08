#include <UDRefl/ReflMngr.h>

#include <set>
#include <array>

using namespace Ubpa::UDRefl;

ReflMngr::ReflMngr() {
	MethodInfo methodinfo_malloc{ {
		[](ArgsView args, void* result_buffer)->Destructor* {
			assert(args.GetParamList().GetParameters().at(0).typeID == ReflMngr::Instance().tregistry.GetID<uint64_t>());
			auto size = args.At(0).As<uint64_t>();
			void* buffer = malloc(size);
			buffer_get<void*>(result_buffer, 0) = buffer;
			return nullptr;
		},
		{ // ResultDesc
			tregistry.GetID<void*>(),
			sizeof(void*),
			alignof(void*),
		},
		{{// ParamList
			{ // size
				tregistry.GetID<uint64_t>(),
				sizeof(uint64_t),
				alignof(uint64_t),
				nregistry.GetID("size")
			}
		}}
	} };
	MethodInfo methodinfo_free{ {
		[](ArgsView args, void*)->Destructor* {
			assert(args.GetParamList().GetParameters().at(0).typeID == ReflMngr::Instance().tregistry.GetID<void*>());
			auto ptr = args.At(0).As<void*>();
			free(ptr);
			return nullptr;
		},
		{}, // ResultDesc
		{{// ParamList
			{ // ptr
				tregistry.GetID<void*>(),
				sizeof(void*),
				alignof(void*),
				nregistry.GetID("ptr")
			}
		}}
	} };
	MethodInfo methodinfo_aligned_malloc{ {
		[](ArgsView args, void* result_buffer)->Destructor* {
			assert(args.GetParamList().GetParameters().at(0).typeID == ReflMngr::Instance().tregistry.GetID<uint64_t>());
			assert(args.GetParamList().GetParameters().at(1).typeID == ReflMngr::Instance().tregistry.GetID<uint64_t>());
			auto size = args.At(0).As<uint64_t>();
			auto alignment = args.At(1).As<uint64_t>();
			void* buffer =
#if defined(WIN32) || defined(_WINDOWS)
				_aligned_malloc(size, alignment)
#else
				aligned_alloc(size, alignment)
#endif // defined(WIN32) || defined(_WINDOWS)
				;
			buffer_get<void*>(result_buffer, 0) = buffer;
			return nullptr;
		},
		{ // ResultDesc
			tregistry.GetID<void*>(),
			sizeof(void*),
			alignof(void*),
		},
		{{// ParamList
			{ // size
				tregistry.GetID<uint64_t>(),
				sizeof(uint64_t),
				alignof(uint64_t),
				nregistry.GetID("size")
			},
			{ // alignment
				tregistry.GetID<uint64_t>(),
				sizeof(uint64_t),
				alignof(uint64_t),
				nregistry.GetID("alignment")
			},
		}}
	} };
	MethodInfo methodinfo_aligned_free{ {
		[](ArgsView args, void*)->Destructor* {
			assert(args.GetParamList().GetParameters().at(0).typeID == ReflMngr::Instance().tregistry.GetID<void*>());
			auto ptr = args.At(0).As<void*>();

#if defined(WIN32) || defined(_WINDOWS)
			_aligned_free(ptr);
#else
			free(ptr);
#endif // defined(WIN32) || defined(_WINDOWS)

			return nullptr;
		},
		{}, // ResultDesc
		{{// ParamList
			{ // ptr
				tregistry.GetID<void*>(),
				sizeof(void*),
				alignof(void*),
				nregistry.GetID("ptr")
			}
		}}
	} };

	TypeInfo global{
		0,
		0,
		{}, // fieldinfos
		{ // methodinfos
			{nregistry.GetID(NameRegistry::Meta::malloc), std::move(methodinfo_malloc)},
			{nregistry.GetID(NameRegistry::Meta::free), std::move(methodinfo_free)},
			{nregistry.GetID(NameRegistry::Meta::aligned_malloc), std::move(methodinfo_aligned_malloc)},
			{nregistry.GetID(NameRegistry::Meta::aligned_free), std::move(methodinfo_aligned_free)},
		},
	};

	typeinfos.emplace(tregistry.GetID(TypeRegistry::Meta::global), std::move(global));
}

void* ReflMngr::Malloc(uint64_t size) const {
	std::array argsTypeIDs = { tregistry.GetID<uint64_t>() };

	std::uint8_t args_buffer[sizeof(uint64_t)];
	buffer_get<uint64_t>(args_buffer, 0) = size;

	std::uint8_t result_buffer[sizeof(void*)];

	auto result = Invoke(nregistry.GetID(NameRegistry::Meta::malloc), argsTypeIDs, args_buffer, result_buffer);

	if (result.success)
		return buffer_get<void*>(result_buffer, 0);
	else
		return nullptr;
}

bool ReflMngr::Free(void* ptr) const {
	std::array argsTypeIDs = { tregistry.GetID<void*>() };

	std::uint8_t args_buffer[sizeof(void*)];
	buffer_get<void*>(args_buffer, 0) = ptr;

	auto result = Invoke(nregistry.GetID(NameRegistry::Meta::free), argsTypeIDs, args_buffer);

	return result.success;
}

void* ReflMngr::AlignedMalloc(uint64_t size, uint64_t alignment) const {
	std::array argsTypeIDs = { tregistry.GetID<uint64_t>(),tregistry.GetID<uint64_t>() };

	std::uint8_t args_buffer[2 * sizeof(uint64_t)];
	buffer_get<uint64_t>(args_buffer, 0) = size;
	buffer_get<uint64_t>(args_buffer, sizeof(uint64_t)) = alignment;

	std::uint8_t result_buffer[sizeof(void*)];
	
	auto result = Invoke(nregistry.GetID(NameRegistry::Meta::aligned_malloc), argsTypeIDs, args_buffer, result_buffer);

	if (result.success)
		return buffer_get<void*>(result_buffer, 0);
	else
		return nullptr;
}

bool ReflMngr::AlignedFree(void* ptr) const {
	std::array argsTypeIDs = { tregistry.GetID<void*>() };

	std::uint8_t args_buffer[sizeof(void*)];
	buffer_get<void*>(args_buffer, 0) = ptr;

	auto result = Invoke(nregistry.GetID(NameRegistry::Meta::aligned_free), argsTypeIDs, args_buffer);

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

ObjectPtr ReflMngr::RWVar(ObjectPtr obj, NameID fieldID) const noexcept {
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

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
	return typeinfo.IsInvocable(nregistry.GetID(NameRegistry::Meta::ctor), argTypeIDs);
}

bool ReflMngr::IsDestructible(TypeID typeID) const noexcept {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	return typeinfo.IsConstInvocable(nregistry.GetID(NameRegistry::Meta::dtor), {});
}

bool ReflMngr::Construct(ObjectPtr obj, Span<TypeID> argTypeIDs, void* args_buffer) const {
	assert(obj);
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	auto rst = typeinfo.Invoke(obj, nregistry.GetID(NameRegistry::Meta::ctor), argTypeIDs, args_buffer, nullptr);
	return rst.success;
}

bool ReflMngr::Destruct(ConstObjectPtr obj) const {
	assert(obj);
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	auto rst = typeinfo.Invoke(obj, nregistry.GetID(NameRegistry::Meta::ctor), {}, nullptr, nullptr);
	return rst.success;
}

static void ReflMngr_ForEachTypeID(
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

		ReflMngr_ForEachTypeID(baseID, func, visitedVBs);
	}
}

void ReflMngr::ForEachTypeID(TypeID typeID, const std::function<void(TypeID)>& func) const {
	std::set<TypeID> visitedVBs;
	ReflMngr_ForEachTypeID(typeID, func, visitedVBs);
}

static void ReflMngr_ForEachTypeInfo(
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

		ReflMngr_ForEachTypeInfo(baseID, func, visitedVBs);
	}
}

void ReflMngr::ForEachType(TypeID typeID, const std::function<void(Type)>& func) const {
	std::set<TypeID> visitedVBs;
	ReflMngr_ForEachTypeInfo(typeID, func, visitedVBs);
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

static void ReflMngr_ForEachRWVar(
	ObjectPtr obj,
	const std::function<void(Type, Field, ObjectPtr)>& func,
	std::set<TypeID>& visitedVBs )
{
	if (!obj)
		return;

	auto target = ReflMngr::Instance().typeinfos.find(obj.GetID());

	if (target == ReflMngr::Instance().typeinfos.end())
		return;

	const auto& typeinfo = target->second;

	for (const auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
		if (!fieldInfo.fieldptr.IsConst())
			func({ obj.GetID(), typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.Map(obj));
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		if (baseinfo.IsVirtual()) {
			if (visitedVBs.find(baseID) != visitedVBs.end())
				continue;
			visitedVBs.insert(baseID);
		}

		ReflMngr_ForEachRWVar(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, func, visitedVBs);
	}
}

void ReflMngr::ForEachRWVar(
	ObjectPtr obj,
	const std::function<void(Type, Field, ObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	ReflMngr_ForEachRWVar(obj, func, visitedVBs);
}

static void ReflMngr_ForEachRVar(
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

		ReflMngr_ForEachRVar(ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, func, visitedVBs);
	}
}

void ReflMngr::ForEachRVar(
	ConstObjectPtr obj,
	const std::function<void(Type, Field, ConstObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	ReflMngr_ForEachRVar(obj, func, visitedVBs);
}
