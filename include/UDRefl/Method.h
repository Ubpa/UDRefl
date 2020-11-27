#pragma once

#include "Object.h"

#include <UContainer/Span.h>

#include <vector>
#include <functional>
#include <any>
#include <variant>

namespace Ubpa::UDRefl {
	struct Parameter {
		size_t typeID;
		size_t size;
		size_t alignment;
		size_t nameID{ static_cast<size_t>(-1) };
	};

	class ParamList {
	public:
		ParamList(std::vector<Parameter> params = {});
		size_t GetBufferSize() const noexcept { return size; }
		size_t GetBufferAlignment() const noexcept { return alignment; }
		const std::vector<size_t>& GetOffsets() const noexcept { return offsets; }
		const std::vector<Parameter>& GetParameters() const noexcept { return params; }
		bool IsConpatibleWith(Span<size_t> typeIDs) const noexcept;
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
				reinterpret_cast<std::uint8_t*>(buffer) + paramList.GetOffsets()[idx]
			};
		}
	private:
		void* buffer;
		const ParamList& paramList;
	};

	class Method {
	public:
		enum class Type {
			OBJECT_VARIABLE,
			OBJECT_CONST,
			STATIC,
		};

		using ObjectVariableFunction = std::any(void*, ArgsView);
		using ObjectConstFunction = std::any(const void*, ArgsView);
		using StaticFunction = std::any(ArgsView);

		Method(ObjectVariableFunction* func, ParamList paramList = {}) noexcept :
			type{ Type::OBJECT_VARIABLE },
			func_object_variable{ func },
			paramList{ std::move(paramList) }
		{
			assert(func);
		}

		Method(ObjectConstFunction* func, ParamList paramList = {}) noexcept :
			type{ Type::OBJECT_CONST },
			func_object_const{ func },
			paramList{ std::move(paramList) }
		{
			assert(func);
		}

		Method(StaticFunction* func, ParamList paramList = {}) noexcept :
			type{ Type::STATIC },
			func_static{ func },
			paramList{ std::move(paramList) }
		{
			assert(func);
		}

		const ParamList& GetParamList() const noexcept { return paramList; }
		Type GetType() const noexcept { return type; }

		std::any Invoke(void* obj, void* buffer) const {
			ArgsView args = { buffer,paramList };
			switch (type)
			{
			case Type::OBJECT_VARIABLE:
				return func_object_variable(obj, args);
			case Type::OBJECT_CONST:
				return func_object_const(obj, args);
			case Type::STATIC:
				return func_static(args);
			default:
				assert(false);
				return {};
			}
		};

		std::any Invoke(const void* obj, void* buffer) const {
			ArgsView args = { buffer,paramList };
			switch (type)
			{
			case Type::OBJECT_CONST:
				return func_object_const(obj, args);
			case Type::STATIC:
				return func_static(args);
			default:
				assert(false);
				return {};
			}
		};

		std::any Invoke(void* buffer) const {
			ArgsView args = { buffer,paramList };
			switch (type)
			{
			case Type::STATIC:
				return func_static(args);
			default:
				assert(false);
				return {};
			}
		};

		std::any Invoke_Static(void* buffer) const {
			assert(type == Type::STATIC);
			return func_static({ buffer,paramList });
		};

	private:
		Type type;
		union {
			std::any(*func_object_variable)(void*, ArgsView);
			std::any(*func_object_const)(const void*, ArgsView);
			std::any(*func_static)(ArgsView);
		};
		ParamList paramList;
	};

	struct InvokeResult {
		bool success{ false };
		std::any value;
	};
}
