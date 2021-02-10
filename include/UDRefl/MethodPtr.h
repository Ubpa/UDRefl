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
		using MemberVariableFunction = void(      void*, void*, ArgsView);
		using MemberConstFunction    = void(const void*, void*, ArgsView);
		using StaticFunction         = void(             void*, ArgsView);

		MethodPtr(MemberVariableFunction*               func, Type result_type = Type_of<void>, ParamList paramList = {});
		MethodPtr(MemberConstFunction*                  func, Type result_type = Type_of<void>, ParamList paramList = {});
		MethodPtr(StaticFunction*                       func, Type result_type = Type_of<void>, ParamList paramList = {});
		MethodPtr(std::function<MemberVariableFunction> func, Type result_type = Type_of<void>, ParamList paramList = {});
		MethodPtr(std::function<MemberConstFunction>    func, Type result_type = Type_of<void>, ParamList paramList = {});
		MethodPtr(std::function<StaticFunction>         func, Type result_type = Type_of<void>, ParamList paramList = {});

		bool IsMemberVariable() const noexcept { return func.index() == 0 || func.index() == 3; }
		bool IsMemberConst   () const noexcept { return func.index() == 1 || func.index() == 4; }
		bool IsStatic        () const noexcept { return func.index() == 2 || func.index() == 5; }

		MethodFlag GetMethodFlag() const noexcept;

		const ParamList& GetParamList() const noexcept { return paramList; }
		const Type& GetResultType() const noexcept { return result_type; }

		bool IsDistinguishableWith(const MethodPtr& rhs) const noexcept {
			return func.index() != rhs.func.index() || paramList != rhs.paramList;
		}

		void Invoke(      void* obj, void* result_buffer, ArgPtrBuffer argptr_buffer) const;
		void Invoke(const void* obj, void* result_buffer, ArgPtrBuffer argptr_buffer) const;
		void Invoke(                 void* result_buffer, ArgPtrBuffer argptr_buffer) const;

	private:
		std::variant<
			MemberVariableFunction*,
			MemberConstFunction*,
			StaticFunction*,
			std::function<MemberVariableFunction>,
			std::function<MemberConstFunction>,
			std::function<StaticFunction>
		> func;
		Type result_type;
		ParamList paramList;
	};
}
