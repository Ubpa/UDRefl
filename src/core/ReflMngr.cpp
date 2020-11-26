#include <UDRefl/ReflMngr.h>

using namespace Ubpa::UDRefl;

bool ReflMngr::IsCastable(size_t fromID, size_t toID) const noexcept {
	if (fromID == toID)
		return true;

	if (toID == static_cast<size_t>(-1))
		return true;

	{ // <from> as base
		auto typetarget = typeinfos.find(toID);
		if (typetarget != typeinfos.end()) {
			auto basetarget = typetarget->second.baseinfos.find(fromID);
			if (basetarget != typetarget->second.baseinfos.end())
				return true;
		}
	}

	{ // <from> as derived
		auto typetarget = typeinfos.find(fromID);
		if (typetarget != typeinfos.end()) {
			auto basetarget = typetarget->second.baseinfos.find(toID);
			if (basetarget != typetarget->second.baseinfos.end()) {
				return true;
			}
		}
	}

	return false;
}

ObjectPtr ReflMngr::Cast(ObjectPtr obj, size_t typeID) const noexcept {
	if (obj.GetID() == typeID)
		return obj;

	if (obj.GetPtr() == nullptr)
		return obj;

	if (typeID == static_cast<size_t>(-1))
		return nullptr;

	{ // as base
		auto typetarget = typeinfos.find(typeID);
		if (typetarget != typeinfos.end()) {
			auto basetarget = typetarget->second.baseinfos.find(obj.GetID());
			if (basetarget != typetarget->second.baseinfos.end()) {
				return {
					typeID,
					backward_offset(obj, basetarget->second.offset)
				};
			}
		}
	}

	{ // as derived
		auto typetarget = typeinfos.find(obj.GetID());
		if (typetarget != typeinfos.end()) {
			auto basetarget = typetarget->second.baseinfos.find(typeID);
			if (basetarget != typetarget->second.baseinfos.end()) {
				return {
					typeID,
					forward_offset(obj, basetarget->second.offset)
				};
			}
		}
	}

	return nullptr;
}

// non-const
ObjectPtr ReflMngr::RWField(ObjectPtr obj, size_t fieldID) const noexcept {
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	auto ptr = typeinfo.RWField(obj, fieldID);
	if (ptr)
		return ptr;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RWField(ObjectPtr{ baseID, forward_offset(obj, baseinfo.offset) }, fieldID);
		if (bptr)
			return bptr;
	}

	return nullptr;
}

// non-const + const
ConstObjectPtr ReflMngr::RField(ConstObjectPtr obj, size_t fieldID) const noexcept {
	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	auto ptr = typeinfo.RField(obj, fieldID);
	if (ptr)
		return ptr;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RField(ConstObjectPtr{ baseID, forward_offset(obj, baseinfo.offset) }, fieldID);
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

InvokeResult ReflMngr::Invoke(size_t typeID, size_t methodID, Span<size_t> argTypeIDs, void* buffer) const {
	auto typetarget = typeinfos.find(typeID);

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto rst = typeinfo.Invoke(methodID, argTypeIDs, buffer);

	if (rst.success)
		return rst;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = Invoke(baseID, methodID, argTypeIDs, buffer);
		if (rst.success)
			return rst;
	}

	return {};
}

InvokeResult ReflMngr::Invoke(ConstObjectPtr obj, size_t methodID, Span<size_t> argTypeIDs, void* buffer) const {
	auto typetarget = typeinfos.find(obj.GetID());

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto rst = typeinfo.Invoke(obj, methodID, argTypeIDs, buffer);

	if (rst.success)
		return rst;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = Invoke(
			ConstObjectPtr{ baseID, forward_offset(obj, baseinfo.offset) },
			methodID, argTypeIDs, buffer
		);
		if (rst.success)
			return rst;
	}

	return {};
}

InvokeResult ReflMngr::Invoke(ObjectPtr obj, size_t methodID, Span<size_t> argTypeIDs, void* buffer) const {
	auto typetarget = typeinfos.find(obj.GetID());

	if (typetarget == typeinfos.end())
		return {};

	const auto& typeinfo = typetarget->second;

	auto rst = typeinfo.Invoke(obj, methodID, argTypeIDs, buffer);

	if (rst.success)
		return rst;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto rst = Invoke(
			ObjectPtr{ baseID, forward_offset(obj, baseinfo.offset) },
			methodID, argTypeIDs, buffer
		);
		if (rst.success)
			return rst;
	}

	return {};
}
