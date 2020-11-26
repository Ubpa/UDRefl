#include <UDRefl/TypeInfo.h>

using namespace Ubpa::UDRefl;

bool TypeInfo::IsInvocable(size_t methodID, ObjectPtr obj, Span<size_t> argTypeIDs) const noexcept {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.paramList.IsConpatibleWith(argTypeIDs))
			return true;
	}
	return IsInvocable(methodID, ConstObjectPtr{ obj }, argTypeIDs);
}

bool TypeInfo::IsInvocable(size_t methodID, ConstObjectPtr obj, Span<size_t> argTypeIDs) const noexcept {
	auto target = cmethodinfos.find(methodID);
	size_t num = cmethodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.paramList.IsConpatibleWith(argTypeIDs))
			return true;
	}
	return false;
}

std::any TypeInfo::Invoke(size_t methodID, ObjectPtr obj, Span<size_t> argTypeIDs, void* buffer) const {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.paramList.IsConpatibleWith(argTypeIDs))
			return target->second.method.Invoke(obj, buffer);
	}
	return Invoke(methodID, ConstObjectPtr{ obj }, argTypeIDs, buffer);
}

std::any TypeInfo::Invoke(size_t methodID, ConstObjectPtr obj, Span<size_t> argTypeIDs, void* buffer) const {
	auto target = cmethodinfos.find(methodID);
	size_t num = cmethodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.paramList.IsConpatibleWith(argTypeIDs))
			return target->second.method.Invoke(obj, buffer);
	}
	assert(false);
	return {};
}
