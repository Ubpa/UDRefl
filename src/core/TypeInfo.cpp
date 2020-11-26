#include <UDRefl/TypeInfo.h>

using namespace Ubpa::UDRefl;

bool TypeInfo::IsInvocable(size_t methodID, ObjectPtr obj, Span<size_t> typeIDs) const noexcept {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.paramList.IsConpatibleWith(typeIDs))
			return true;
	}
	return IsInvocable(methodID, ConstObjectPtr{ obj }, typeIDs);
}

bool TypeInfo::IsInvocable(size_t methodID, ConstObjectPtr obj, Span<size_t> typeIDs) const noexcept {
	auto target = cmethodinfos.find(methodID);
	size_t num = cmethodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.paramList.IsConpatibleWith(typeIDs))
			return true;
	}
	return false;
}

std::any TypeInfo::Invoke(size_t methodID, ObjectPtr obj, void* buffer, Span<size_t> typeIDs) const {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.paramList.IsConpatibleWith(typeIDs))
			return target->second.method.Invoke(obj, buffer);
	}
	return Invoke(methodID, ConstObjectPtr{ obj }, buffer, typeIDs);
}

std::any TypeInfo::Invoke(size_t methodID, ConstObjectPtr obj, void* buffer, Span<size_t> typeIDs) const {
	auto target = cmethodinfos.find(methodID);
	size_t num = cmethodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.paramList.IsConpatibleWith(typeIDs))
			return target->second.method.Invoke(obj, buffer);
	}
	assert(false);
	return {};
}
