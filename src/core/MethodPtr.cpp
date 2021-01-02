#include <UDRefl/MethodPtr.h>

using namespace Ubpa::UDRefl;

Destructor MethodPtr::Invoke(void* obj, void* result_buffer, ArgsBuffer args_buffer) const {
	return std::visit([=](const auto& f) {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, MemberVariableFunction*>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, MemberConstFunction*>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, StaticFunction*>)
			return f(     result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			return f(     result_buffer, { args_buffer,paramList });
		else
			static_assert(always_false<Func>);
	}, func);
};

Destructor MethodPtr::Invoke(const void* obj, void* result_buffer, ArgsBuffer args_buffer) const {
	return std::visit([=](const auto& f)->Destructor {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, MemberVariableFunction*>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, MemberConstFunction*>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, StaticFunction*>)
			return f(     result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			return f(     result_buffer, { args_buffer,paramList });
		else
			static_assert(always_false<Func>);
	}, func);
};

Destructor MethodPtr::Invoke(void* result_buffer, ArgsBuffer args_buffer) const {
	return std::visit([=](const auto& f)->Destructor {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, MemberVariableFunction*>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, MemberConstFunction*>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, StaticFunction*>)
			return f(result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>) {
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
			static_assert(always_false<Func>);
	}, func);
};
