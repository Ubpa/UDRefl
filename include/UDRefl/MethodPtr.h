#pragma once

#include "Object.h"

#include <vector>
#include <variant>

namespace Ubpa::UDRefl {
	using ParamList = std::vector<Type>;
	
	class ArgsView {
	public:
		ArgsView(ArgPtrBuffer buffer, const ParamList& paramList) : buffer{ buffer }, paramList{ paramList }{}
		ArgPtrBuffer GetBuffer() const noexcept { return buffer; }
		const ParamList& GetParamList() const noexcept { return paramList; }

		ObjectView At(size_t idx) const noexcept {
			assert(idx < paramList.size());
			return { paramList[idx], buffer[idx] };
		}
	private:
		ArgPtrBuffer buffer;
		const ParamList& paramList;
	};

	class MethodPtr {
	public:
		using Func = std::function<void(void*, void*, ArgsView)>;

		MethodPtr() = default;
		MethodPtr(Func func, MethodFlag flag, Type result_type = Type_of<void>, ParamList paramList = {});

		MethodFlag GetMethodFlag() const noexcept { return flag; }
		const Type& GetResultType() const noexcept { return result_type; }
		const ParamList& GetParamList() const noexcept { return paramList; }

		bool IsDistinguishableWith(const MethodPtr& rhs) const noexcept { return flag != rhs.flag || paramList != rhs.paramList; }

		void Invoke(      void* obj, void* result_buffer, ArgPtrBuffer argptr_buffer) const;
		void Invoke(const void* obj, void* result_buffer, ArgPtrBuffer argptr_buffer) const;
		void Invoke(                 void* result_buffer, ArgPtrBuffer argptr_buffer) const;

	private:
		Func func;
		MethodFlag flag;
		Type result_type;
		ParamList paramList;
	};
}
