#include <UDRefl/MethodPtr.h>

using namespace Ubpa::UDRefl;

ParamList::ParamList(std::vector<Parameter> params) :
	params{ params }
{
	offsets.reserve(params.size());

	size_t curOffset = 0;
	for (const auto& param : params) {
		if (param.alignment > alignment)
			alignment = param.alignment;

		curOffset = (curOffset + param.alignment - 1) / param.alignment;
		offsets.push_back(curOffset);
		curOffset += param.size;
	}

	size = curOffset;
}

bool ParamList::IsConpatibleWith(Span<const TypeID> typeIDs) const noexcept {
	if (params.size() != typeIDs.size())
		return false;

	for (size_t i = 0; i < params.size(); i++) {
		if (params[i].typeID != typeIDs[i]) {
			if ((params[i].typeID != TypeID::of<ConstObjectPtr> || typeIDs[i] != TypeID::of<ObjectPtr>)
				&& (params[i].typeID != TypeID::of<const SharedObject&> || typeIDs[i] != TypeID::of<SharedObject>))
				return false;
		}
	}

	return true;
}

bool ParamList::operator==(const ParamList& rhs) const noexcept {
	if (size != rhs.size)
		return false;
	if (alignment != rhs.alignment)
		return false;
	if (params.size() != rhs.params.size())
		return false;
	for (size_t i = 0; i < params.size(); i++) {
		if (params[i].typeID != rhs.params[i].typeID)
			return false;
	}
	return true;
}

Destructor MethodPtr::Invoke(void* obj, void* result_buffer, void* args_buffer) const {
	return std::visit([=](const auto& f) {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			return f(     result_buffer, { args_buffer,paramList });
		else
			static_assert(false);
		}, func);
};

Destructor MethodPtr::Invoke(const void* obj, void* result_buffer, void* args_buffer) const {
	return std::visit([=](const auto& f)->Destructor {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			return f(     result_buffer, { args_buffer,paramList });
		else
			static_assert(false);
		}, func);
};

Destructor MethodPtr::Invoke(void* result_buffer, void* args_buffer) const {
	return std::visit([=](const auto& f)->Destructor {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			return f(result_buffer, { args_buffer,paramList });
		else
			static_assert(false);
		}, func);
};
