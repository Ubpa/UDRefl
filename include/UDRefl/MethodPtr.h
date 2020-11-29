#pragma once

#include "Object.h"
#include "Util.h"

#include <UContainer/Span.h>

#include <vector>
#include <functional>
#include <any>
#include <variant>

namespace Ubpa::UDRefl {
	struct Parameter {
		TypeID typeID;
		size_t size;
		size_t alignment;
		NameID nameID;
	};

	struct ResultDesc {
		TypeID typeID;
		size_t size{ 0 };
		size_t alignment{ 1 };
	};

	class ParamList {
	public:
		ParamList(std::vector<Parameter> params = {});
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
		ObjectPtr At(size_t idx) const noexcept {
			assert(idx < paramList.GetParameters().size());
			return {
				paramList.GetParameters()[idx].typeID,
				forward_offset(buffer, paramList.GetOffsets()[idx])
			};
		}
	private:
		void* buffer;
		const ParamList& paramList;
	};

	class MethodPtr {
	public:
		enum class Mode {
			OBJECT_VARIABLE,
			OBJECT_CONST,
			STATIC,
		};

		template<typename T>
		static MethodPtr GenerateDefaultConstructor() {
			return { static_cast<ObjectVariableFunction*>(
				[](void* obj, ArgsView, void*) -> Destructor* {
					new(obj)T;
					return nullptr;
				}
			) };
		}

		template<typename T>
		static MethodPtr GenerateDestructor() {
			return { static_cast<ObjectConstFunction*>(
				[](const void* obj, ArgsView, void*) -> Destructor* {
					reinterpret_cast<const T*>(obj)->~T();
					return nullptr;
				}
			) };
		}

		using ObjectVariableFunction = std::add_pointer_t<Destructor>(void*, ArgsView, void*);
		using ObjectConstFunction    = std::add_pointer_t<Destructor>(const void*, ArgsView, void*);
		using StaticFunction         = std::add_pointer_t<Destructor>(ArgsView, void*);

		MethodPtr(ObjectVariableFunction* func, ParamList paramList = {}, ResultDesc resultDesc = {}) noexcept :
			mode{ Mode::OBJECT_VARIABLE },
			func_object_variable{ func },
			resultDesc{ std::move(resultDesc) },
			paramList{ std::move(paramList) }
		{
			assert(func);
		}

		MethodPtr(ObjectConstFunction* func, ParamList paramList = {}, ResultDesc resultDesc = {}) noexcept :
			mode{ Mode::OBJECT_CONST },
			func_object_const{ func },
			resultDesc{ std::move(resultDesc) },
			paramList{ std::move(paramList) }
		{
			assert(func);
		}

		MethodPtr(StaticFunction* func, ParamList paramList = {}, ResultDesc resultDesc = {}) noexcept :
			mode{ Mode::STATIC },
			func_static{ func },
			resultDesc{ std::move(resultDesc) },
			paramList{ std::move(paramList) }
		{
			assert(func);
		}

		Mode GetMode() const noexcept { return mode; }
		const ParamList& GetParamList() const noexcept { return paramList; }
		const ResultDesc& GetResultDesc() const noexcept { return resultDesc; }

		Destructor* Invoke(void* obj, void* args_buffer, void* result_buffer) const {
			ArgsView args = { args_buffer,paramList };
			switch (mode)
			{
			case Mode::OBJECT_VARIABLE:
				return func_object_variable(obj, args, result_buffer);
			case Mode::OBJECT_CONST:
				return func_object_const(obj, args, result_buffer);
			case Mode::STATIC:
				return func_static(args, result_buffer);
			default:
				assert(false);
				return {};
			}
		};

		Destructor* Invoke(const void* obj, void* args_buffer, void* result_buffer) const {
			ArgsView args = { args_buffer,paramList };
			switch (mode)
			{
			case Mode::OBJECT_CONST:
				return func_object_const(obj, args, result_buffer);
			case Mode::STATIC:
				return func_static(args, result_buffer);
			default:
				assert(false);
				return nullptr;
			}
		};

		Destructor* Invoke(void* args_buffer, void* result_buffer) const {
			ArgsView args = { args_buffer,paramList };
			switch (mode)
			{
			case Mode::STATIC:
				return func_static(args, result_buffer);
			default:
				assert(false);
				return nullptr;
			}
		};

		Destructor* Invoke_Static(void* args_buffer, void* result_buffer) const {
			assert(mode == Mode::STATIC);
			return func_static({ args_buffer,paramList }, result_buffer);
		};

	private:
		Mode mode;
		union {
			ObjectVariableFunction* func_object_variable;
			ObjectConstFunction* func_object_const;
			StaticFunction* func_static;
		};
		ResultDesc resultDesc;
		ParamList paramList;
	};

	struct InvokeResult {
		bool success{ false };
		TypeID resultID;
		Destructor* destructor{ nullptr };
	};
}
