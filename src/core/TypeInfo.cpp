#include <UDRefl/TypeInfo.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa::UDRefl;

ObjectPtr TypeInfo::RWField(size_t fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	if (target->second.fieldptr.GetType() != FieldPtr::Type::STATIC_VARIABLE)
		return nullptr;

	return target->second.fieldptr.Map_StaticVariable();
}

ConstObjectPtr TypeInfo::RField(size_t fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	if (!target->second.fieldptr.IsStatic())
		return nullptr;

	return target->second.fieldptr.Map();
}

ObjectPtr TypeInfo::RWField(void* obj, size_t fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	if (target->second.fieldptr.IsConst())
		return nullptr;

	return target->second.fieldptr.Map(obj);
}

ConstObjectPtr TypeInfo::RField(const void* obj, size_t fieldID) const noexcept {
	auto target = fieldinfos.find(fieldID);
	if (target == fieldinfos.end())
		return nullptr;

	return target->second.fieldptr.Map(obj);
}

bool TypeInfo::IsStaticInvocable(size_t methodID, Span<size_t> argTypeIDs) const noexcept {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.GetType() == Method::Type::STATIC
			&& target->second.method.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}
	return false;
}

bool TypeInfo::IsConstInvocable(size_t methodID, Span<size_t> argTypeIDs) const noexcept {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.GetType() != Method::Type::OBJECT_VARIABLE
			&& target->second.method.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}
	return false;
}

bool TypeInfo::IsInvocable(size_t methodID, Span<size_t> argTypeIDs) const noexcept {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.GetParamList().IsConpatibleWith(argTypeIDs))
			return true;
	}
	return false;
}

InvokeResult TypeInfo::Invoke(size_t methodID, Span<size_t> argTypeIDs, void* args_buffer, void* result_buffer) const {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.GetType() == Method::Type::STATIC
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

InvokeResult TypeInfo::Invoke(const void* obj, size_t methodID, Span<size_t> argTypeIDs, void* args_buffer, void* result_buffer) const {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);
	for (size_t i = 0; i < num; ++i, ++target) {
		if (target->second.method.GetType() != Method::Type::OBJECT_VARIABLE
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

InvokeResult TypeInfo::Invoke(void* obj, size_t methodID, Span<size_t> argTypeIDs, void* args_buffer, void* result_buffer) const {
	auto target = methodinfos.find(methodID);
	size_t num = methodinfos.count(methodID);

	{ // first: object variable and static
		auto iter = target;
		for (size_t i = 0; i < num; ++i, ++iter) {
			if (iter->second.method.GetType() != Method::Type::OBJECT_CONST
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
			if (iter->second.method.GetType() == Method::Type::OBJECT_CONST
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
