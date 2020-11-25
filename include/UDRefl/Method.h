#pragma once

#include "Object.h"

#include <vector>
#include <functional>
#include <any>

namespace Ubpa::UDRefl {
	struct Parameter {
		size_t typeID;
		size_t nameID;
		bool isConst;
		size_t size;
		size_t alignment;
	};

	class ParamList {
	public:
		ParamList(std::vector<Parameter> params = {});
		size_t GetBufferSize() const noexcept { return size; }
		size_t GetBufferAlignment() const noexcept { return alignment; }
		const std::vector<size_t>& GetOffsets() const noexcept { return offsets; }
		const std::vector<Parameter>& GetParameters() const noexcept { return params; }
	private:
		size_t size{ 0 };
		size_t alignment{ 1 };
		std::vector<size_t> offsets;
		std::vector<Parameter> params;
	};
	
	class ArgsView {
	public:
		ArgsView(void* buffer, const ParamList& paramList) : buffer{ buffer }, paramList{ paramList }{}
		const ParamList& GetParamList() const noexcept { return paramList; }
		ObjectPtr NonConstAt(size_t idx) const noexcept {
			assert(idx < paramList.GetParameters().size());
			const auto& param = paramList.GetParameters()[idx];
			assert(!param.isConst);
			return { param.typeID, reinterpret_cast<std::uint8_t*>(buffer) + paramList.GetOffsets()[idx] };
		}
		ConstObjectPtr ConstAt(size_t idx) const noexcept {
			assert(idx < paramList.GetParameters().size());
			const auto& param = paramList.GetParameters()[idx];
			assert(param.isConst);
			return { param.typeID, reinterpret_cast<std::uint8_t*>(buffer) + paramList.GetOffsets()[idx] };
		}
	private:
		void* buffer;
		const ParamList& paramList;
	};

	struct Method {
		ParamList paramList;
		std::function<std::any(ObjectPtr, ArgsView)> func;
		std::unordered_map<size_t, std::any> attrs;
		std::any Invoke(ObjectPtr obj = {}, void* buffer = nullptr) { return func(obj, { buffer, paramList }); };
	};

	struct ConstMethod {
		ParamList paramList;
		std::function<std::any(ConstObjectPtr, ArgsView)> func;
		std::unordered_map<size_t, std::any> attrs;
		std::any Invoke(ConstObjectPtr obj = {}, void* buffer = nullptr) { return func(obj, { buffer, paramList }); };
	};
}
