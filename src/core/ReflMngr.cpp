#include <UDRefl/ReflMngr.h>

using namespace Ubpa::UDRefl;

ObjectPtr ReflMngr::StaticCast_DerivedToBase(ObjectPtr obj, size_t typeID) const noexcept {
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

ObjectPtr ReflMngr::StaticCast_BaseToDerived(ObjectPtr obj, size_t typeID) const noexcept {
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

ObjectPtr ReflMngr::DynamicCast_BaseToDerived(ObjectPtr obj, size_t typeID) const noexcept {
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

ObjectPtr ReflMngr::StaticCast(ObjectPtr obj, size_t typeID) const noexcept {
	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetID() != static_cast<size_t>(-1))
		return ptr_d2b;

	auto ptr_b2d = StaticCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetID() != static_cast<size_t>(-1))
		return ptr_b2d;

	return nullptr;
}

ObjectPtr ReflMngr::DynamicCast(ObjectPtr obj, size_t typeID) const noexcept {
	auto ptr_b2d = DynamicCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetID() != static_cast<size_t>(-1))
		return ptr_b2d;

	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetID() != static_cast<size_t>(-1))
		return ptr_d2b;

	return nullptr;
}

ObjectPtr ReflMngr::RWField(ObjectPtr obj, size_t fieldID) const noexcept {
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	auto ptr = typeinfo.RWField(obj, fieldID);
	if (ptr)
		return ptr;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RWField(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, fieldID);
		if (bptr)
			return bptr;
	}

	return nullptr;
}

ConstObjectPtr ReflMngr::RField(ConstObjectPtr obj, size_t fieldID) const noexcept {
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	auto ptr = typeinfo.RField(obj, fieldID);
	if (ptr)
		return ptr;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RField(ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj) }, fieldID);
		if (bptr)
			return bptr;
	}

	return nullptr;
}

bool ReflMngr::IsStaticInvocable(size_t typeID, size_t methodID, Span<size_t> argTypeIDs) const noexcept {
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

bool ReflMngr::IsConstInvocable(size_t typeID, size_t methodID, Span<size_t> argTypeIDs) const noexcept {
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

bool ReflMngr::IsInvocable(size_t typeID, size_t methodID, Span<size_t> argTypeIDs) const noexcept {
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

InvokeResult ReflMngr::Invoke(size_t typeID, size_t methodID, Span<size_t> argTypeIDs, void* args_buffer, void* result_buffer) const {
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

InvokeResult ReflMngr::Invoke(ConstObjectPtr obj, size_t methodID, Span<size_t> argTypeIDs, void* args_buffer, void* result_buffer) const {
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

InvokeResult ReflMngr::Invoke(ObjectPtr obj, size_t methodID, Span<size_t> argTypeIDs, void* args_buffer, void* result_buffer) const {
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
