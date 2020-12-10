#include <UDRefl/TypeInfo.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa::UDRefl;

ObjectPtr TypeInfo::RWVar(NameID fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	if (target->second.fieldptr.GetMode() != FieldPtr::Mode::STATIC_VARIABLE)
		return nullptr;

	return target->second.fieldptr.Map_StaticVariable();
}

ConstObjectPtr TypeInfo::RVar(NameID fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	if (!target->second.fieldptr.IsStatic())
		return nullptr;

	return target->second.fieldptr.Map();
}

ObjectPtr TypeInfo::RWVar(void* obj, NameID fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	if (target->second.fieldptr.IsConst())
		return nullptr;

	return target->second.fieldptr.Map(obj);
}

ConstObjectPtr TypeInfo::RVar(const void* obj, NameID fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	return target->second.fieldptr.Map(obj);
}

bool TypeInfo::IsStaticInvocable(NameID methodID, Span<TypeID> argTypeIDs) const noexcept {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.methodptr.IsStatic()
			&& target->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}
	return false;
}

bool TypeInfo::IsConstInvocable(NameID methodID, Span<TypeID> argTypeIDs) const noexcept {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (!target->second.methodptr.IsMemberVariable()
			&& target->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}
	return false;
}

bool TypeInfo::IsInvocable(NameID methodID, Span<TypeID> argTypeIDs) const noexcept {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}
	return false;
}

InvokeResult TypeInfo::Invoke(NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer, void* result_buffer) const {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.methodptr.IsStatic()
			&& target->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
		{
			return {
				true,
				target->second.methodptr.GetResultDesc().typeID,
				target->second.methodptr.Invoke_Static(args_buffer, result_buffer)
			};
		}
	}
	return {};
}

InvokeResult TypeInfo::Invoke(const void* obj, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer, void* result_buffer) const {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (!target->second.methodptr.IsMemberVariable()
			&& target->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
		{
			return {
				true,
				target->second.methodptr.GetResultDesc().typeID,
				target->second.methodptr.Invoke(obj, args_buffer, result_buffer)
			};
		}
	}
	return {};
}

InvokeResult TypeInfo::Invoke(void* obj, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer, void* result_buffer) const {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);

	{ // first: object variable and static
		auto iter = target;
		for (size_t i = 0; i < num; ++i, ++iter) {
			if (!iter->second.methodptr.IsMemberConst()
				&& iter->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			{
				return {
					true,
					iter->second.methodptr.GetResultDesc().typeID,
					iter->second.methodptr.Invoke(obj, args_buffer, result_buffer)
				};
			}
		}
	}

	{ // second: object const
		auto iter = target;
		for (size_t i = 0; i < num; ++i, ++iter) {
			if (iter->second.methodptr.IsMemberConst()
				&& iter->second.methodptr.GetParamList().IsConpatibleWith(argTypeIDs))
			{
				return {
					true,
					iter->second.methodptr.GetResultDesc().typeID,
					iter->second.methodptr.Invoke(obj, args_buffer, result_buffer)
				};
			}
		}
	}

	return {};
}
