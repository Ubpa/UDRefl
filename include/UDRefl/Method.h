#pragma once

#include "Object.h"

#include <UContainer/Span.h>

#include <vector>
#include <functional>
#include <any>

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

	struct Method {
		ParamList paramList;
		std::function<std::any(void*, ArgsView)> func;
		std::any Invoke(void* obj, void* buffer) const {
			assert(obj != nullptr);
			return func(obj, { buffer, paramList });
		};
	};

	struct ConstMethod {
		ParamList paramList;
		std::function<std::any(const void*, ArgsView)> func;
		std::any Invoke(const void* obj, void* buffer) const {
			assert(obj != nullptr);
			return func(obj, { buffer, paramList });
		};
	};

	struct StaticMethod {
		ParamList paramList;
		std::function<std::any(ArgsView)> func;
		std::any Invoke(void* buffer) const {
			return func({ buffer, paramList });
		};
	};

	struct InvokeResult {
		bool success{ false };
		std::any value;
	};
}
