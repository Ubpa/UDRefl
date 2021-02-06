#include <UDRefl/MethodPtr.h>

using namespace Ubpa::UDRefl;

MethodPtr::MethodPtr(MemberVariableFunction* func, ResultDesc resultDesc, ParamList paramList) :
	func{ func },
	resultDesc{ std::move(resultDesc) },
	paramList{ std::move(paramList) } { assert(func); }

MethodPtr::MethodPtr(MemberConstFunction* func, ResultDesc resultDesc, ParamList paramList) :
	func{ func },
	resultDesc{ std::move(resultDesc) },
	paramList{ std::move(paramList) } { assert(func); }

MethodPtr::MethodPtr(StaticFunction* func, ResultDesc resultDesc, ParamList paramList) :
	func{ func },
	resultDesc{ std::move(resultDesc) },
	paramList{ std::move(paramList) } { assert(func); }

MethodPtr::MethodPtr(std::function<MemberVariableFunction> func, ResultDesc resultDesc, ParamList paramList) :
	func{ (assert(func), std::move(func)) },
	resultDesc{ std::move(resultDesc) },
	paramList{ std::move(paramList) } {}

MethodPtr::MethodPtr(std::function<MemberConstFunction> func, ResultDesc resultDesc, ParamList paramList) :
	func{ (assert(func), std::move(func)) },
	resultDesc{ std::move(resultDesc) },
	paramList{ std::move(paramList) } {}

MethodPtr::MethodPtr(std::function<StaticFunction> func, ResultDesc resultDesc, ParamList paramList) :
	func{ (assert(func), std::move(func)) },
	resultDesc{ std::move(resultDesc) },
	paramList{ std::move(paramList) } {}

FuncFlag MethodPtr::GetFuncFlag() const noexcept {
	switch (func.index())
	{
	case 0: [[fallthrough]];
	case 3:
		return FuncFlag::Variable;
	case 1: [[fallthrough]];
	case 4:
		return FuncFlag::Const;
	case 2:
	case 5:
		return FuncFlag::Static;
	default:
		return FuncFlag::None;
	}
}

Destructor MethodPtr::Invoke(void* obj, void* result_buffer, ArgPtrBuffer argptr_buffer) const {
	return std::visit([=, this](const auto& f) {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, MemberVariableFunction*>)
			return f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, MemberConstFunction*>)
			return f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, StaticFunction*>)
			return f(     result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>)
			return f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
			return f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			return f(     result_buffer, { argptr_buffer,paramList });
		else
			static_assert(always_false<Func>);
	}, func);
};

Destructor MethodPtr::Invoke(const void* obj, void* result_buffer, ArgPtrBuffer argptr_buffer) const {
	return std::visit([=, this](const auto& f)->Destructor {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, MemberVariableFunction*>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, MemberConstFunction*>)
			return f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, StaticFunction*>)
			return f(     result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
			return f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			return f(     result_buffer, { argptr_buffer,paramList });
		else
			static_assert(always_false<Func>);
	}, func);
};

Destructor MethodPtr::Invoke(void* result_buffer, ArgPtrBuffer argptr_buffer) const {
	return std::visit([=, this](const auto& f)->Destructor {
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
			return f(result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			return f(result_buffer, { argptr_buffer,paramList });
		else
			static_assert(always_false<Func>);
	}, func);
};
