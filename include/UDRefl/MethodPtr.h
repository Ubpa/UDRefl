#pragma once

#include "Object.h"

//#include <vector>

namespace Ubpa::UDRefl {
	class ArgsView {
	public:
		constexpr ArgsView() noexcept : buffer{ nullptr } {}
		constexpr ArgsView(ArgPtrBuffer buffer, std::span<const Type> argTypes) noexcept : buffer{ buffer }, argTypes{ argTypes }{}
		constexpr std::size_t Size() const noexcept { return argTypes.size(); }
		constexpr ObjectView operator[](size_t idx) const noexcept { return { argTypes[idx], buffer[idx] }; }
	private:
		ArgPtrBuffer buffer;
		std::span<const Type> argTypes;
	};

	using ParamList = std::vector<Type>;

	class MethodPtr {
	public:
		using Func = std::function<void(void*, void*, ArgsView)>;

		MethodPtr() = default;
		MethodPtr(Func func, MethodFlag flag, Type result_type = Type_of<void>, ParamList paramList = {});

		MethodFlag GetMethodFlag() const noexcept { return flag; }
		const Type& GetResultType() const noexcept { return result_type; }
		const ParamList& GetParamList() const noexcept { return paramList; }

		bool IsDistinguishableWith(const MethodPtr& rhs) const noexcept { return flag != rhs.flag || paramList != rhs.paramList; }

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
