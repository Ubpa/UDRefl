#include <UDRefl/TypeInfo.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa::UDRefl;

ObjectPtr TypeInfo::RWField(NameID fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	if (target->second.fieldptr.GetMode() != FieldPtr::Mode::STATIC_VARIABLE)
		return nullptr;

	return target->second.fieldptr.Map_StaticVariable();
}

ConstObjectPtr TypeInfo::RField(NameID fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	if (!target->second.fieldptr.IsStatic())
		return nullptr;

	return target->second.fieldptr.Map();
}

ObjectPtr TypeInfo::RWField(void* obj, NameID fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	if (target->second.fieldptr.IsConst())
		return nullptr;

	return target->second.fieldptr.Map(obj);
}

ConstObjectPtr TypeInfo::RField(const void* obj, NameID fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	return target->second.fieldptr.Map(obj);
}

bool TypeInfo::IsStaticInvocable(NameID methodID, Span<TypeID> argTypeIDs) const noexcept {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.GetMode() == Method::Mode::STATIC
			&& target->second.method.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}
	return false;
}

bool TypeInfo::IsConstInvocable(NameID methodID, Span<TypeID> argTypeIDs) const noexcept {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.GetMode() != Method::Mode::OBJECT_VARIABLE
			&& target->second.method.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}
	return false;
}

bool TypeInfo::IsInvocable(NameID methodID, Span<TypeID> argTypeIDs) const noexcept {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}
	return false;
}

InvokeResult TypeInfo::Invoke(NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer, void* result_buffer) const {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.GetMode() == Method::Mode::STATIC
			&& target->second.method.GetParamList().IsConpatibleWith(argTypeIDs))
		{
			return {
				true,
				target->second.method.GetResultDesc().typeID,
				target->second.method.Invoke_Static(args_buffer, result_buffer)
			};
		}
	}
	return {};
}

InvokeResult TypeInfo::Invoke(const void* obj, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer, void* result_buffer) const {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.GetMode() != Method::Mode::OBJECT_VARIABLE
			&& target->second.method.GetParamList().IsConpatibleWith(argTypeIDs))
		{
			return {
				true,
				target->second.method.GetResultDesc().typeID,
				target->second.method.Invoke(obj, args_buffer, result_buffer)
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
			if (iter->second.method.GetMode() != Method::Mode::OBJECT_CONST
				&& iter->second.method.GetParamList().IsConpatibleWith(argTypeIDs))
			{
				return {
					true,
					iter->second.method.GetResultDesc().typeID,
					iter->second.method.Invoke(obj, args_buffer, result_buffer)
				};
			}
		}
	}

	{ // second: object const
		auto iter = target;
		for (size_t i = 0; i < num; ++i, ++iter) {
			if (iter->second.method.GetMode() == Method::Mode::OBJECT_CONST
				&& iter->second.method.GetParamList().IsConpatibleWith(argTypeIDs))
			{
				return {
					true,
					iter->second.method.GetResultDesc().typeID,
					iter->second.method.Invoke(obj, args_buffer, result_buffer)
				};
			}
		}
	}

	return {};
}
