#pragma once

#include "ObjectPtr.h"
#include "Util.h"

#include <UTemplate/Func.h>
#include <UContainer/Span.h>

#include <vector>
#include <functional>
#include <variant>

namespace Ubpa::UDRefl {
	struct ResultDesc {
		TypeID typeID;
		size_t size{ 0 };
		size_t alignment{ 1 };
	};

	struct Parameter {
		TypeID typeID;
		size_t size;
		size_t alignment;
		NameID nameID;
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
		template<typename T>
		static MethodPtr GenerateDefaultConstructor() noexcept;

		template<typename T>
		static MethodPtr GenerateDestructor() noexcept;

		using ObjectVariableFunction = Destructor(void*, ArgsView, void*);
		using ObjectConstFunction    = Destructor(const void*, ArgsView, void*);
		using StaticFunction         = Destructor(ArgsView, void*);

		MethodPtr(std::function<ObjectVariableFunction> func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			func{ (assert(func), std::move(func)) },
			resultDesc{ std::move(resultDesc) },
			paramList{ std::move(paramList) } {}

		MethodPtr(std::function<ObjectConstFunction> func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			func{ (assert(func), std::move(func)) },
			resultDesc{ std::move(resultDesc) },
			paramList{ std::move(paramList) } {}

		MethodPtr(std::function<StaticFunction> func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			func{ (assert(func), std::move(func)) },
			resultDesc{ std::move(resultDesc) },
			paramList{ std::move(paramList) } {}

		MethodPtr(ObjectVariableFunction* func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			MethodPtr{ std::function<ObjectVariableFunction>{func}, std::move(resultDesc), std::move(paramList) }
		{ assert(func); }

		MethodPtr(ObjectConstFunction* func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			MethodPtr{ std::function<ObjectConstFunction>{func}, std::move(resultDesc), std::move(paramList) }
		{ assert(func); }

		MethodPtr(StaticFunction* func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			MethodPtr{ std::function<StaticFunction>{func}, std::move(resultDesc), std::move(paramList) }
		{ assert(func); }

		template<typename Lambda>
		MethodPtr(Lambda func, ResultDesc resultDesc = {}, ParamList paramList = {}) noexcept :
			MethodPtr{ DecayLambda(func), std::move(resultDesc), std::move(paramList) }
		{ assert(func); }

		bool IsObjectVariable() const noexcept { return func.index() == 0; }
		bool IsObjectConst   () const noexcept { return func.index() == 1; }
		bool IsStatic        () const noexcept { return func.index() == 2; }

		const ParamList&  GetParamList() const noexcept { return paramList; }
		const ResultDesc& GetResultDesc() const noexcept { return resultDesc; }

		Destructor Invoke(void* obj, void* args_buffer, void* result_buffer) const {
			return std::visit([=](const auto& f) {
				using Func = std::decay_t<decltype(f)>;
				if constexpr (std::is_same_v<Func, std::function<ObjectVariableFunction>>)
					return f(obj, { args_buffer,paramList }, result_buffer);
				else if constexpr (std::is_same_v<Func, std::function<ObjectConstFunction>>)
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
				if constexpr (std::is_same_v<Func, std::function<ObjectVariableFunction>>) {
					assert(false);
					return Destructor{};
				}
				else if constexpr (std::is_same_v<Func, std::function<ObjectConstFunction>>)
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
				if constexpr (std::is_same_v<Func, std::function<ObjectVariableFunction>>) {
					assert(false);
					return Destructor{};
				}
				else if constexpr (std::is_same_v<Func, std::function<ObjectConstFunction>>) {
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
			std::function<ObjectVariableFunction>, // object variable
			std::function<ObjectConstFunction>,    // object const
			std::function<StaticFunction>          // static
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
