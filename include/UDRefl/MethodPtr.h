#pragma once

#include "ObjectPtr.h"
#include "Util.h"

#include <UContainer/Span.h>

#include <vector>
#include <variant>

namespace Ubpa::UDRefl {
	struct ResultDesc {
		TypeID typeID;
		size_t size;
		size_t alignment;

		constexpr bool IsVoid() const noexcept {
			return typeID == TypeID::of<void>;
		}
	};

	struct Parameter {
		TypeID typeID;
		size_t size;
		size_t alignment;
	};

	class ParamList {
	public:
		ParamList() noexcept = default;
		ParamList(std::vector<Parameter> params);
		size_t GetBufferSize() const noexcept { return size; }
		size_t GetBufferAlignment() const noexcept { return alignment; }
		const std::vector<size_t>& GetOffsets() const noexcept { return offsets; }
		const std::vector<Parameter>& GetParameters() const noexcept { return params; }
		bool IsConpatibleWith(Span<TypeID> typeIDs) const noexcept;
		bool operator==(const ParamList& rhs) const noexcept;
		bool operator!=(const ParamList& rhs) const noexcept { return ! operator==(rhs); }
	private:
		size_t size{ 0 };
		size_t alignment{ 1 };
		std::vector<size_t> offsets;
		std::vector<Parameter> params;
	};
	
	class ArgsView {
	public:
		ArgsView(void* buffer, const ParamList& paramList) : buffer{ buffer }, paramList{ paramList }{}
		void* GetBuffer() const noexcept { return buffer; }
		const ParamList& GetParamList() const noexcept { return paramList; }
		ObjectPtr At(size_t idx) const noexcept;
	private:
		void* buffer;
		const ParamList& paramList;
	};

	class MethodPtr {
	public:
		using MemberVariableFunction = Destructor(void*, ArgsView, void*);
		using MemberConstFunction    = Destructor(const void*, ArgsView, void*);
		using StaticFunction         = Destructor(ArgsView, void*);

		MethodPtr(std::function<MemberVariableFunction> func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			func{ (assert(func), std::move(func)) },
			resultDesc{ std::move(resultDesc) },
			paramList{ std::move(paramList) } {}

		MethodPtr(std::function<MemberConstFunction> func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			func{ (assert(func), std::move(func)) },
			resultDesc{ std::move(resultDesc) },
			paramList{ std::move(paramList) } {}

		MethodPtr(std::function<StaticFunction> func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			func{ (assert(func), std::move(func)) },
			resultDesc{ std::move(resultDesc) },
			paramList{ std::move(paramList) } {}

		MethodPtr(MemberVariableFunction* func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			MethodPtr{ std::function<MemberVariableFunction>{func}, std::move(resultDesc), std::move(paramList) }
		{ assert(func); }

		MethodPtr(MemberConstFunction* func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			MethodPtr{ std::function<MemberConstFunction>{func}, std::move(resultDesc), std::move(paramList) }
		{ assert(func); }

		MethodPtr(StaticFunction* func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			MethodPtr{ std::function<StaticFunction>{func}, std::move(resultDesc), std::move(paramList) }
		{ assert(func); }

		bool IsMemberVariable() const noexcept { return func.index() == 0; }
		bool IsMemberConst   () const noexcept { return func.index() == 1; }
		bool IsStatic        () const noexcept { return func.index() == 2; }

		const ParamList&  GetParamList() const noexcept { return paramList; }
		const ResultDesc& GetResultDesc() const noexcept { return resultDesc; }

		bool IsDistinguishableWith(const MethodPtr& rhs) const noexcept {
			return func.index() != rhs.func.index() ||
				paramList != rhs.paramList;
		}

		Destructor Invoke(void* obj, void* args_buffer, void* result_buffer) const {
			return std::visit([=](const auto& f) {
				using Func = std::decay_t<decltype(f)>;
				if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>)
					return f(obj, { args_buffer,paramList }, result_buffer);
				else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
					return f(obj, { args_buffer,paramList }, result_buffer);
				else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
					return f({ args_buffer,paramList }, result_buffer);
				else
					static_assert(false);
			}, func);
		};

		Destructor Invoke(const void* obj, void* args_buffer, void* result_buffer) const {
			return std::visit([=](const auto& f) {
				using Func = std::decay_t<decltype(f)>;
				if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>) {
					assert(false);
					return Destructor{};
				}
				else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
					return f(obj, { args_buffer,paramList }, result_buffer);
				else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
					return f({ args_buffer,paramList }, result_buffer);
				else
					static_assert(false);
			}, func);
		};

		Destructor Invoke(void* args_buffer, void* result_buffer) const {
			return std::visit([=](const auto& f) {
				using Func = std::decay_t<decltype(f)>;
				if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>) {
					assert(false);
					return Destructor{};
				}
				else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>) {
					assert(false);
					return Destructor{};
				}
				else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
					return f({ args_buffer,paramList }, result_buffer);
				else
					static_assert(false);
			}, func);
		};

		Destructor Invoke_Static(void* args_buffer, void* result_buffer) const {
			assert(IsStatic());
			return std::get<std::function<StaticFunction>>(func)({ args_buffer, paramList }, result_buffer);
		};

	private:
		std::variant<
			std::function<MemberVariableFunction>,
			std::function<MemberConstFunction>,
			std::function<StaticFunction>
		> func;
		ResultDesc resultDesc;
		ParamList paramList;
	};

	struct InvokeResult {
		bool success{ false };
		TypeID resultID;
		Destructor destructor;

		template<typename T>
		T Move(void* result_buffer);
	};
}

#include "details/MethodPtr.inl"
