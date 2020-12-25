#include <UDRefl/ReflMngr.h>

#include <set>

#if defined(_WIN32) || defined(_WIN64)
#include <malloc.h>
#endif

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
				if (!func({ obj.GetID(), typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.RWVar(obj.GetPtr())))
					return false;
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachRWVar(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, func, visitedVBs))
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
			if (!func({ obj.GetID(), typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.RVar(obj.GetPtr())))
				return false;
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachRVar(ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, func, visitedVBs))
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
				if (!func({ typeID, typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.RWVar()))
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
				if (!func({ typeID, typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.RVar()))
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

	DeleteFunc GenerateDeleteFunc(Destructor&& dtor, std::pmr::memory_resource* result_rsrc, size_t size, size_t alignment) {
		assert(size > 0);
		if (dtor) {
			return [d = std::move(dtor), result_rsrc, size, alignment](void* ptr) {
				d(ptr);
				result_rsrc->deallocate(ptr, size, alignment);
			};
		}
		else // !dtor
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

#if defined(_WIN32) || defined(_WIN64)
	#define UBPA_UDREFL_ALIGN_MALLOC _aligned_malloc
#else
	#define UBPA_UDREFL_ALIGN_MALLOC aligned_alloc
#endif // defined(_WIN32) || defined(_WIN64)
	MethodInfo methodinfo_aligned_malloc{ GenerateMethodPtr<&UBPA_UDREFL_ALIGN_MALLOC>() };
#undef UBPA_UDREFL_ALIGN_MALLOC

#if defined(_WIN32) || defined(_WIN64)
#define UBPA_UDREFL_ALIGN_FREE _aligned_free
#else
#define UBPA_UDREFL_ALIGN_FREE free
#endif // defined(_WIN32) || defined(_WIN64)
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

	RegisterTypeAuto<ContainerType>();
	AddField<ContainerType::Unknown>("Unknown");
	AddField<ContainerType::Array>("Array");
	AddField<ContainerType::Vector>("Vector");
	AddField<ContainerType::Deque>("Deque");
	AddField<ContainerType::ForwardList>("ForwardList");
	AddField<ContainerType::List>("List");
	AddField<ContainerType::Set>("Set");
	AddField<ContainerType::Map>("Map");
	AddField<ContainerType::UnorderedSet>("UnorderedSet");
	AddField<ContainerType::UnorderedMap>("UnorderedMap");
	AddField<ContainerType::Stack>("Stack");
	AddField<ContainerType::Queue>("Queue");
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
	TypeID ID{ name };

	auto target = typeinfos.find(ID);
	if (target != typeinfos.end())
		return ID;

	tregistry.Register(ID, name);
	typeinfos.emplace_hint(target, ID, TypeInfo{ size,alignment });

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

bool ReflMngr::AddAttr(TypeID typeID, const Attr& attr) {
	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return false;
	auto& typeinfo = ttarget->second;
	auto atarget = typeinfo.attrs.find(attr);
	if (atarget != typeinfo.attrs.end())
		return false;
	typeinfo.attrs.emplace_hint(atarget, attr);
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

ObjectPtr ReflMngr::StaticCast_DerivedToBase(ObjectPtr obj, TypeID typeID) const {
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
		auto ptr = StaticCast_DerivedToBase(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, typeID);
		if (ptr.GetID())
			return ptr;
	}

	return nullptr;
}

ObjectPtr ReflMngr::StaticCast_BaseToDerived(ObjectPtr obj, TypeID typeID) const {
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
			return { baseID, baseinfo.IsVirtual() ? nullptr : baseinfo.StaticCast_BaseToDerived(obj.GetPtr()) };
	}
	
	return nullptr;
}

ObjectPtr ReflMngr::DynamicCast_BaseToDerived(ObjectPtr obj, TypeID typeID) const {
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
		auto ptr = DynamicCast_BaseToDerived(ObjectPtr{ baseID, baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) }, typeID);
		if (ptr.GetID())
			return { baseID, baseinfo.IsPolymorphic() ? baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) : nullptr };
	}

	return nullptr;
}

ObjectPtr ReflMngr::StaticCast(ObjectPtr obj, TypeID typeID) const {
	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetID())
		return ptr_d2b;

	auto ptr_b2d = StaticCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetID())
		return ptr_b2d;

	return nullptr;
}

ObjectPtr ReflMngr::DynamicCast(ObjectPtr obj, TypeID typeID) const {
	auto ptr_b2d = DynamicCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetID())
		return ptr_b2d;

	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetID())
		return ptr_d2b;

	return nullptr;
}

ObjectPtr ReflMngr::RWVar(TypeID typeID, StrID fieldID) {
	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;

	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end() && ftarget->second.fieldptr.IsObject())
		return ftarget->second.fieldptr.RWVar();

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RWVar(baseID, fieldID);
		if (bptr.GetID())
			return bptr;
	}

	return nullptr;
}

ConstObjectPtr ReflMngr::RVar(TypeID typeID, StrID fieldID) const {
	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;


	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end() && ftarget->second.fieldptr.IsObject())
		return ftarget->second.fieldptr.RVar();

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RVar(baseID, fieldID);
		if (bptr.GetID())
			return bptr;
	}

	return nullptr;
}

ObjectPtr ReflMngr::RWVar(ObjectPtr obj, StrID fieldID) {
	auto ttarget = typeinfos.find(obj.GetID());
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;


	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end() && ftarget->second.fieldptr.IsVariable())
		return ftarget->second.fieldptr.RWVar(obj.GetPtr());

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RWVar(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, fieldID);
		if (bptr.GetID())
			return bptr;
	}

	return nullptr;
}

ConstObjectPtr ReflMngr::RVar(ConstObjectPtr obj, StrID fieldID) const {
	auto ttarget = typeinfos.find(obj.GetID());
	if (ttarget == typeinfos.end())
		return nullptr;

	const auto& typeinfo = ttarget->second;

	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end())
		return ftarget->second.fieldptr.RVar(obj.GetPtr());

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RVar(ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, fieldID);
		if (bptr.GetID())
			return bptr;
	}

	return nullptr;
}

ObjectPtr ReflMngr::RWVar(ObjectPtr obj, TypeID baseID, StrID fieldID) {
	auto base = StaticCast_DerivedToBase(obj, baseID);
	if (!base.GetID())
		return nullptr;
	return RWVar(base, fieldID);
}

ConstObjectPtr ReflMngr::RVar(ConstObjectPtr obj, TypeID baseID, StrID fieldID) const {
	auto base = StaticCast_DerivedToBase(obj, baseID);
	if (!base.GetID())
		return nullptr;
	return RVar(base, fieldID);
}

InvocableResult ReflMngr::IsStaticInvocable(
	TypeID typeID,
	StrID methodID,
	Span<const TypeID> argTypeIDs) const
{
	if (GetDereferenceProperty(typeID) != DereferenceProperty::NotReference)
		return IsStaticInvocable(Dereference(typeID), methodID, argTypeIDs);

	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.IsStatic()
			&& mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			return { true, mtarget->second.methodptr.GetResultDesc() };
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = IsStaticInvocable(baseID, methodID, argTypeIDs);
		if (rst)
			return rst;
	}

	return {};
}

InvocableResult ReflMngr::IsConstInvocable(
	TypeID typeID,
	StrID methodID,
	Span<const TypeID> argTypeIDs) const
{
	if (GetDereferenceProperty(typeID) != DereferenceProperty::NotReference)
		return IsConstInvocable(Dereference(typeID), methodID, argTypeIDs);

	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (!mtarget->second.methodptr.IsMemberVariable()
			&& mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			return { true, mtarget->second.methodptr.GetResultDesc() };
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = IsConstInvocable(baseID, methodID, argTypeIDs);
		if(rst)
			return rst;
	}

	return {};
}

InvocableResult ReflMngr::IsInvocable(
	TypeID typeID,
	StrID methodID,
	Span<const TypeID> argTypeIDs) const
{
	if (GetDereferenceProperty(typeID) != DereferenceProperty::NotReference)
		return IsInvocable(Dereference(typeID), methodID, argTypeIDs);

	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto mtarget = typeinfo.methodinfos.find(methodID);
	size_t num = typeinfo.methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			return {true, mtarget->second.methodptr.GetResultDesc()};
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = IsInvocable(baseID, methodID, argTypeIDs);
		if (rst)
			return rst;
	}

	return {};
}

InvokeResult ReflMngr::Invoke(
	TypeID typeID,
	StrID methodID,
	void* result_buffer,
	Span<const TypeID> argTypeIDs,
	void* args_buffer) const
{
	if (GetDereferenceProperty(typeID) != DereferenceProperty::NotReference)
		return Invoke(Dereference(typeID), methodID, result_buffer, argTypeIDs, args_buffer);

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
	auto deref_prop = GetDereferenceProperty(obj.GetID());
	switch (deref_prop)
	{
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		return Invoke(Dereference(obj), methodID, result_buffer, argTypeIDs, args_buffer);
	case Ubpa::UDRefl::DereferenceProperty::Const:
		return Invoke(DereferenceAsConst(obj), methodID, result_buffer, argTypeIDs, args_buffer);
	default:
		break;
	}

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
				mtarget->second.methodptr.Invoke(obj.GetPtr(), result_buffer, args_buffer)
			};
		}
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = Invoke(
			ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
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
	auto deref_prop = GetDereferenceProperty(obj.GetID());
	switch (deref_prop)
	{
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		return Invoke(Dereference(obj), methodID, result_buffer, argTypeIDs, args_buffer);
	case Ubpa::UDRefl::DereferenceProperty::Const:
		return Invoke(DereferenceAsConst(obj), methodID, result_buffer, argTypeIDs, args_buffer);
	default:
		break;
	}

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
					iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, args_buffer)
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
					iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, args_buffer)
				};
			}
		}
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = Invoke(
			ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
			methodID, result_buffer, argTypeIDs, args_buffer
		);
		if (rst.success)
			return rst;
	}

	return {};
}

SharedObject ReflMngr::MInvoke(
	TypeID typeID,
	StrID methodID,
	Span<const TypeID> argTypeIDs,
	void* args_buffer,
	std::pmr::memory_resource* rst_rsrc)
{
	assert(rst_rsrc);
	if (GetDereferenceProperty(typeID) != DereferenceProperty::NotReference)
		return MInvoke(Dereference(typeID), methodID, argTypeIDs, args_buffer, rst_rsrc);

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
				mtarget->second.methodptr.Invoke_Static(nullptr, args_buffer);
				return {
					{rst_desc.typeID, nullptr},
					[](void* ptr) { assert(ptr); }
				};
			}
			else if (type_name_is_reference(tregistry.Nameof(rst_desc.typeID))) {
				std::uint8_t buffer[sizeof(void*)];
				mtarget->second.methodptr.Invoke_Static(buffer, args_buffer);
				return {
					{rst_desc.typeID, buffer_as<void*>(buffer)},
					[](void* ptr) { assert(ptr); }
				};
			}
			else {
				void* result_buffer = rst_rsrc->allocate(rst_desc.size, rst_desc.alignment);
				auto dtor = mtarget->second.methodptr.Invoke_Static(result_buffer, args_buffer);
				return {
					{rst_desc.typeID, result_buffer},
					details::GenerateDeleteFunc(std::move(dtor), rst_rsrc, rst_desc.size, rst_desc.alignment)
				};
			}
		}
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = MInvoke(baseID, methodID, argTypeIDs, args_buffer, rst_rsrc);
		if (rst.GetID())
			return rst;
	}

	return nullptr;
}

SharedObject ReflMngr::MInvoke(
	ConstObjectPtr obj,
	StrID methodID,
	Span<const TypeID> argTypeIDs,
	void* args_buffer,
	std::pmr::memory_resource* rst_rsrc)
{
	assert(rst_rsrc);
	auto deref_prop = GetDereferenceProperty(obj.GetID());
	switch (deref_prop)
	{
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		return MInvoke(Dereference(obj), methodID, argTypeIDs, args_buffer, rst_rsrc);
	case Ubpa::UDRefl::DereferenceProperty::Const:
		return MInvoke(DereferenceAsConst(obj), methodID, argTypeIDs, args_buffer, rst_rsrc);
	default:
		break;
	}
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
				auto dtor = mtarget->second.methodptr.Invoke(obj.GetPtr(), nullptr, args_buffer);
				return {
					{rst_desc.typeID, nullptr},
					[](void* ptr) { assert(!ptr); }
				};
			}
			else if (type_name_is_reference(tregistry.Nameof(rst_desc.typeID))) {
				std::uint8_t buffer[sizeof(void*)];
				mtarget->second.methodptr.Invoke(obj.GetPtr(), buffer, args_buffer);
				return {
					{rst_desc.typeID, buffer_as<void*>(buffer)},
					[](void* ptr) { assert(ptr); }
				};
			}
			else {
				void* result_buffer = rst_rsrc->allocate(rst_desc.size, rst_desc.alignment);
				auto dtor = mtarget->second.methodptr.Invoke(obj.GetPtr(), result_buffer, args_buffer);
				return {
					{rst_desc.typeID, result_buffer},
					details::GenerateDeleteFunc(std::move(dtor), rst_rsrc, rst_desc.size, rst_desc.alignment)
				};
			}
		}
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = MInvoke(
			ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
			methodID, argTypeIDs, args_buffer, rst_rsrc
		);
		if (rst.GetID())
			return rst;
	}

	return nullptr;
}

SharedObject ReflMngr::MInvoke(
	ObjectPtr obj,
	StrID methodID,
	Span<const TypeID> argTypeIDs,
	void* args_buffer,
	std::pmr::memory_resource* rst_rsrc)
{
	assert(rst_rsrc);
	auto deref_prop = GetDereferenceProperty(obj.GetID());
	switch (deref_prop)
	{
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		return MInvoke(Dereference(obj), methodID, argTypeIDs, args_buffer, rst_rsrc);
	case Ubpa::UDRefl::DereferenceProperty::Const:
		return MInvoke(DereferenceAsConst(obj), methodID, argTypeIDs, args_buffer, rst_rsrc);
	default:
		break;
	}
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
					auto dtor = iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, args_buffer);
					return {
						{rst_desc.typeID, nullptr},
						[](void* ptr) { assert(!ptr); }
					};
				}
				else if (type_name_is_reference(tregistry.Nameof(rst_desc.typeID))) {
					std::uint8_t buffer[sizeof(void*)];
					iter->second.methodptr.Invoke(obj.GetPtr(), buffer, args_buffer);
					return {
						{rst_desc.typeID, buffer_as<void*>(buffer)},
						[](void* ptr) { assert(ptr); }
					};
				}
				else {
					void* result_buffer = rst_rsrc->allocate(rst_desc.size, rst_desc.alignment);
					auto dtor = iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, args_buffer);
					return {
						{rst_desc.typeID, result_buffer},
						details::GenerateDeleteFunc(std::move(dtor), rst_rsrc, rst_desc.size, rst_desc.alignment)
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
					auto dtor = iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, args_buffer);
					return {
						{rst_desc.typeID, nullptr},
						[](void* ptr) { assert(!ptr); }
					};
				}
				else if (type_name_is_reference(tregistry.Nameof(rst_desc.typeID))) {
					std::uint8_t buffer[sizeof(void*)];
					iter->second.methodptr.Invoke(obj.GetPtr(), buffer, args_buffer);
					return {
						{rst_desc.typeID, buffer_as<void*>(buffer)},
						[](void* ptr) { assert(ptr); }
					};
				}
				else {
					void* result_buffer = rst_rsrc->allocate(rst_desc.size, rst_desc.alignment);
					auto dtor = iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, args_buffer);
					return {
						{rst_desc.typeID, result_buffer},
						details::GenerateDeleteFunc(std::move(dtor), rst_rsrc, rst_desc.size, rst_desc.alignment)
					};
				}
			}
		}
	}

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = MInvoke(
			ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
			methodID, argTypeIDs, args_buffer, rst_rsrc
		);
		if (rst.GetID())
			return rst;
	}

	return nullptr;
}

bool ReflMngr::IsConstructible(TypeID typeID, Span<const TypeID> argTypeIDs) const {
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

bool ReflMngr::IsDestructible(TypeID typeID) const {
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
	if (!obj.GetPtr())
		return false;

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
			mtarget->second.methodptr.Invoke(obj.GetPtr(), nullptr, args_buffer);
			return true;
		}
	}
	return false;
}

bool ReflMngr::Destruct(ConstObjectPtr obj) const {
	if (!obj.GetPtr())
		return false;

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
			mtarget->second.methodptr.Invoke(obj.GetPtr(), nullptr, {});
			return true;
		}
	}
	return false;
}

void ReflMngr::ForEachTypeID(TypeID typeID, const std::function<bool(TypeID)>& func) const {
	std::set<TypeID> visitedVBs;
	details::ForEachTypeID(Dereference(typeID), func, visitedVBs);
}

void ReflMngr::ForEachType(TypeID typeID, const std::function<bool(TypeRef)>& func) const {
	std::set<TypeID> visitedVBs;
	details::ForEachTypeInfo(Dereference(typeID), func, visitedVBs);
}

void ReflMngr::ForEachField(
	TypeID typeID,
	const std::function<bool(TypeRef, FieldRef)>& func) const
{
	ForEachType(typeID, [&func](TypeRef type) {
		for (auto& [fieldID, fieldInfo] : type.info.fieldinfos) {
			if (!func(type, { fieldID, fieldInfo }))
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
	details::ForEachRWVar(Dereference(typeID), func, visitedVBs);
}

void ReflMngr::ForEachRVar(
	TypeID typeID,
	const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	details::ForEachRVar(Dereference(typeID), func, visitedVBs);
}

void ReflMngr::ForEachRWVar(
	ObjectPtr obj,
	const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	switch (GetDereferenceProperty(obj.GetID()))
	{
	case Ubpa::UDRefl::DereferenceProperty::NotReference:
		details::ForEachRWVar(obj, func, visitedVBs);
		break;
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		details::ForEachRWVar(Dereference(obj), func, visitedVBs);
		break;
	case Ubpa::UDRefl::DereferenceProperty::Const:
	default:
		break;
	}
}

void ReflMngr::ForEachRVar(
	ConstObjectPtr obj,
	const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	switch (GetDereferenceProperty(obj.GetID()))
	{
	case Ubpa::UDRefl::DereferenceProperty::NotReference:
		details::ForEachRVar(obj, func, visitedVBs);
		break;
	case Ubpa::UDRefl::DereferenceProperty::Const:
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		details::ForEachRVar(DereferenceAsConst(obj), func, visitedVBs);
		break;
	default:
		break;
	}
}

std::vector<TypeID> ReflMngr::GetTypeIDs(TypeID typeID) {
	std::vector<TypeID> rst;
	ForEachTypeID(typeID, [&rst](TypeID typeID) {
		rst.push_back(typeID);
		return true;
	});
	return rst;
}

std::vector<TypeRef> ReflMngr::GetTypes(TypeID typeID) {
	std::vector<TypeRef> rst;
	ForEachType(typeID, [&rst](TypeRef type) {
		rst.push_back(type);
		return true;
	});
	return rst;
}

std::vector<TypeFieldRef> ReflMngr::GetTypeFields(TypeID typeID) {
	std::vector<TypeFieldRef> rst;
	ForEachField(typeID, [&rst](TypeRef type, FieldRef field) {
		rst.emplace_back(TypeFieldRef{ type, field });
		return true;
	});
	return rst;
}

std::vector<FieldRef> ReflMngr::GetFields(TypeID typeID) {
	std::vector<FieldRef> rst;
	ForEachField(typeID, [&rst](TypeRef type, FieldRef field) {
		rst.push_back(field);
		return true;
	});
	return rst;
}

std::vector<TypeMethodRef> ReflMngr::GetTypeMethods(TypeID typeID) {
	std::vector<TypeMethodRef> rst;
	ForEachMethod(typeID, [&rst](TypeRef type, MethodRef field) {
		rst.emplace_back(TypeMethodRef{ type, field });
		return true;
	});
	return rst;
}

std::vector<MethodRef> ReflMngr::GetMethods(TypeID typeID) {
	std::vector<MethodRef> rst;
	ForEachMethod(typeID, [&rst](TypeRef type, MethodRef field) {
		rst.push_back(field);
		return true;
	});
	return rst;
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> ReflMngr::GetTypeFieldRWVars(TypeID typeID) {
	std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> rst;
	ForEachRWVar(typeID, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::vector<ObjectPtr> ReflMngr::GetRWVars(TypeID typeID) {
	std::vector<ObjectPtr> rst;
	ForEachRWVar(typeID, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}

std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> ReflMngr::GetTypeFieldRVars(TypeID typeID) {
	std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> rst;
	ForEachRVar(typeID, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::vector<ConstObjectPtr> ReflMngr::GetRVars(TypeID typeID) {
	std::vector<ConstObjectPtr> rst;
	ForEachRVar(typeID, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> ReflMngr::GetTypeFieldRWVars(ObjectPtr obj) {
	std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> rst;
	ForEachRWVar(obj, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::vector<ObjectPtr> ReflMngr::GetRWVars(ObjectPtr obj) {
	std::vector<ObjectPtr> rst;
	ForEachRWVar(obj, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}


std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> ReflMngr::GetTypeFieldRVars(ConstObjectPtr obj) {
	std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> rst;
	ForEachRVar(obj, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::vector<ConstObjectPtr> ReflMngr::GetRVars(ConstObjectPtr obj) {
	std::vector<ConstObjectPtr> rst;
	ForEachRVar(obj, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}

std::optional<TypeID> ReflMngr::FindTypeID(TypeID typeID, const std::function<bool(TypeID)>& func) const {
	std::optional<TypeID> rst;
	ForEachTypeID(typeID, [&rst, func](TypeID typeID) {
		if (!func(typeID))
			return true;

		rst.emplace(typeID);
		return false; // stop
	});
	return rst;
}

std::optional<TypeRef> ReflMngr::FindType(TypeID typeID, const std::function<bool(TypeRef)>& func) const {
	std::optional<TypeRef> rst;
	ForEachType(typeID, [&rst, func](TypeRef type) {
		if (!func(type))
			return true;

		rst.emplace(type);
		return false; // stop
	});
	return rst;
}

std::optional<FieldRef> ReflMngr::FindField(TypeID typeID, const std::function<bool(FieldRef)>& func) const {
	std::optional<FieldRef> rst;
	ForEachField(typeID, [&rst, func](TypeRef type, FieldRef field) {
		if (!func(field))
			return true;

		rst.emplace(field);
		return false; // stop
	});
	return rst;
}

std::optional<MethodRef> ReflMngr::FindMethod(TypeID typeID, const std::function<bool(MethodRef)>& func) const {
	std::optional<MethodRef> rst;
	ForEachMethod(typeID, [&rst, func](TypeRef type, MethodRef method) {
		if (!func(method))
			return true;

		rst.emplace(method);
		return false; // stop
	});
	return rst;
}

ObjectPtr ReflMngr::FindRWVar(TypeID typeID, const std::function<bool(ObjectPtr)>& func) const {
	ObjectPtr rst;
	ForEachRWVar(typeID, [&rst, func](TypeRef type, FieldRef method, ObjectPtr obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

ConstObjectPtr ReflMngr::FindRVar(TypeID typeID, const std::function<bool(ConstObjectPtr)>& func) const {
	ConstObjectPtr rst;
	ForEachRVar(typeID, [&rst, func](TypeRef type, FieldRef method, ConstObjectPtr obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

ObjectPtr ReflMngr::FindRWVar(ObjectPtr obj, const std::function<bool(ObjectPtr)>& func) const {
	ObjectPtr rst;
	ForEachRWVar(obj, [&rst, func](TypeRef type, FieldRef method, ObjectPtr obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

ConstObjectPtr ReflMngr::FindRVar(ConstObjectPtr obj, const std::function<bool(ConstObjectPtr)>& func) const {
	ConstObjectPtr rst;
	ForEachRVar(obj, [&rst, func](TypeRef type, FieldRef method, ConstObjectPtr obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

DereferenceProperty ReflMngr::GetDereferenceProperty(TypeID ID) const {
	auto name = tregistry.Nameof(ID);

	if (!type_name_is_reference(name))
		return DereferenceProperty::NotReference;

	auto unref_name = type_name_remove_reference(name);

	return type_name_is_const(unref_name) ? DereferenceProperty::Const : DereferenceProperty::Variable;
}

TypeID ReflMngr::Dereference(TypeID ID) const {
	auto name = tregistry.Nameof(ID);

	if (!type_name_is_reference(name))
		return ID;

	std::string_view ele_name = type_name_remove_reference(name);

	auto rst_name = type_name_remove_cv(ele_name);

	return rst_name;
}

ObjectPtr ReflMngr::Dereference(ConstObjectPtr ref_obj) const {
	if (!ref_obj.GetPtr())
		return nullptr;

	auto name = tregistry.Nameof(ref_obj.GetID());

	if (!type_name_is_reference(name))
		return nullptr;

	std::string_view rst_name = type_name_remove_reference(name);

	if (type_name_is_const(rst_name))
		return nullptr;

	assert(!type_name_is_volatile(rst_name));

	return { TypeID{ rst_name }, const_cast<void*>(ref_obj.GetPtr()) };
}

ConstObjectPtr ReflMngr::DereferenceAsConst(ConstObjectPtr ref_obj) const {
	if (!ref_obj.Valid())
		return nullptr;

	auto name = tregistry.Nameof(ref_obj.GetID());

	if (!type_name_is_reference(name))
		return nullptr;

	std::string_view ele_name = type_name_remove_reference(name);

	auto rst_name = type_name_remove_const(ele_name);
	assert(!type_name_is_volatile(rst_name));

	return { TypeID{ rst_name }, ref_obj.GetPtr() };
}

TypeID ReflMngr::AddConstLValueReference(TypeID ID) {
	return tregistry.RegisterAddConstLValueReference(ID);
}

ConstObjectPtr ReflMngr::AddConstLValueReference(ConstObjectPtr obj) {
	TypeID newID = tregistry.RegisterAddConstLValueReference(obj.GetID());
	if (!newID.Valid())
		return {};

	return { newID, obj.GetPtr() };
}
