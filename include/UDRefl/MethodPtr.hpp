#pragma once

#include "Object.hpp"

//#include <vector>

namespace Ubpa::UDRefl {
	using ParamList = std::vector<Type>;

	class UDRefl_core_API MethodPtr {
	public:
		using Func = std::function<void(void*, void*, ArgsView)>;

		MethodPtr() = default;
		MethodPtr(Func func, MethodFlag flag, Type result_type = Type_of<void>, ParamList paramList = {});

		MethodFlag GetMethodFlag() const noexcept { return flag; }
		const Type& GetResultType() const noexcept { return result_type; }
		const ParamList& GetParamList() const noexcept { return paramList; }

		bool IsDistinguishableWith(const MethodPtr& rhs) const noexcept { return flag != rhs.flag || paramList != rhs.paramList; }

		// argTypes[i] == paramList[i] || paramList[i].Is<ObjectView>()
		bool IsMatch(std::span<const Type> argTypes) const noexcept;
		
		void Invoke(      void* obj, void* result_buffer, ArgsView args) const;
		void Invoke(const void* obj, void* result_buffer, ArgsView args) const;
		void Invoke(                 void* result_buffer, ArgsView args) const;

	private:
		Func func;
		MethodFlag flag{ MethodFlag::None };
		Type result_type;
		ParamList paramList;
	};
}
