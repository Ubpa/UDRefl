#include <UDRefl/MethodPtr.h>

using namespace Ubpa::UDRefl;

bool ParamList::IsConpatibleWith(Span<const TypeID> typeIDs) const noexcept {
	if (params.size() != typeIDs.size())
		return false;

	for (size_t i = 0; i < params.size(); i++) {
		if (params[i] != typeIDs[i]) {
			if ((params[i] != TypeID::of<ConstObjectPtr> || typeIDs[i] != TypeID::of<ObjectPtr>)
				&& (params[i] != TypeID::of<SharedConstObject> || typeIDs[i] != TypeID::of<SharedObject>))
				return false;
		}
	}

	return true;
}

bool ParamList::operator==(const ParamList& rhs) const noexcept {
	if (params.size() != rhs.params.size())
		return false;

	for (size_t i = 0; i < params.size(); i++) {
		if (params[i] != rhs.params[i])
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
