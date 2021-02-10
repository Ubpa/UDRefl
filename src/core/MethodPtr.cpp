#include <UDRefl/MethodPtr.h>

using namespace Ubpa::UDRefl;

MethodPtr::MethodPtr(MemberVariableFunction* func, Type result_type, ParamList paramList) :
	func{ func },
	result_type{ std::move(result_type) },
	paramList{ std::move(paramList) } { assert(func); }

MethodPtr::MethodPtr(MemberConstFunction* func, Type result_type, ParamList paramList) :
	func{ func },
	result_type{ std::move(result_type) },
	paramList{ std::move(paramList) } { assert(func); }

MethodPtr::MethodPtr(StaticFunction* func, Type result_type, ParamList paramList) :
	func{ func },
	result_type{ std::move(result_type) },
	paramList{ std::move(paramList) } { assert(func); }

MethodPtr::MethodPtr(std::function<MemberVariableFunction> func, Type result_type, ParamList paramList) :
	func{ (assert(func), std::move(func)) },
	result_type{ std::move(result_type) },
	paramList{ std::move(paramList) } {}

MethodPtr::MethodPtr(std::function<MemberConstFunction> func, Type result_type, ParamList paramList) :
	func{ (assert(func), std::move(func)) },
	result_type{ std::move(result_type) },
	paramList{ std::move(paramList) } {}

MethodPtr::MethodPtr(std::function<StaticFunction> func, Type result_type, ParamList paramList) :
	func{ (assert(func), std::move(func)) },
	result_type{ std::move(result_type) },
	paramList{ std::move(paramList) } {}

MethodFlag MethodPtr::GetMethodFlag() const noexcept {
	switch (func.index())
	{
	case 0: [[fallthrough]];
	case 3:
		return MethodFlag::Variable;
	case 1: [[fallthrough]];
	case 4:
		return MethodFlag::Const;
	case 2:
	case 5:
		return MethodFlag::Static;
	default:
		return MethodFlag::None;
	}
}

void MethodPtr::Invoke(void* obj, void* result_buffer, ArgPtrBuffer argptr_buffer) const {
	std::visit([=, this](const auto& f) {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, MemberVariableFunction*>)
			f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, MemberConstFunction*>)
			f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, StaticFunction*>)
			f(     result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>)
			f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
			f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			f(     result_buffer, { argptr_buffer,paramList });
		else
			static_assert(always_false<Func>);
	}, func);
};

void MethodPtr::Invoke(const void* obj, void* result_buffer, ArgPtrBuffer argptr_buffer) const {
	std::visit([=, this](const auto& f) {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, MemberVariableFunction*>)
			assert(false);
		else if constexpr (std::is_same_v<Func, MemberConstFunction*>)
			f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, StaticFunction*>)
			f(     result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>)
			assert(false);
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
			f(obj, result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			f(     result_buffer, { argptr_buffer,paramList });
		else
			static_assert(always_false<Func>);
	}, func);
};

void MethodPtr::Invoke(void* result_buffer, ArgPtrBuffer argptr_buffer) const {
	std::visit([=, this](const auto& f) {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, MemberVariableFunction*>)
			assert(false);
		else if constexpr (std::is_same_v<Func, MemberConstFunction*>)
			assert(false);
		else if constexpr (std::is_same_v<Func, StaticFunction*>)
			f(result_buffer, { argptr_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>)
			assert(false);
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
			assert(false);
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			f(result_buffer, { argptr_buffer,paramList });
		else
			static_assert(always_false<Func>);
	}, func);
};
