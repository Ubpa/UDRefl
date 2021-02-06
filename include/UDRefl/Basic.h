#pragma once

#include "Util.h"

#include <UTemplate/Type.h>

#include <memory>
#include <cassert>

namespace Ubpa::UDRefl {
	enum class FuncFlag {
		Variable = 0b001,
		Const    = 0b010,
		Static   = 0b100,
		
		All      = 0b111
	};
	UBPA_UDREFL_ENUM_BOOL_OPERATOR_DEFINE(FuncFlag)

	using SharedBuffer = std::shared_ptr<void>;

	struct ResultDesc {
		Type type{ Type_of<void> };
		size_t size{ 0 };
		size_t alignment{ 1 };
	};

	struct InvokeResult {
		bool success{ false };
		Type type;
		Destructor destructor;

		template<typename T>
		T Move(void* result_buffer) noexcept(std::is_reference_v<T> || std::is_nothrow_destructible_v<T> && std::is_nothrow_move_constructible_v<T>) {
			static_assert(!std::is_void_v<T>);

			assert(result_buffer);

			if constexpr (!std::is_reference_v<T> && std::is_default_constructible_v<T>) {
				if (!success || type != Type_of<T>)
					return {};
			}
			else
				assert(success && type == Type_of<T>);

			if constexpr (std::is_reference_v<T>) {
				assert(!destructor);
				return std::forward<T>(*buffer_as<std::add_pointer_t<T>>(result_buffer));
			}
			else {
				T rst = std::move(buffer_as<T>(result_buffer));
				if (destructor)
					destructor(result_buffer);
				return rst;
			}
		}

		constexpr operator bool() const noexcept { return success; }
	};

	struct InvocableResult {
		bool success{ false };
		ResultDesc result_desc;
		constexpr operator bool() const noexcept { return success; }
	};

	struct TypeInfo;
	struct FieldInfo;
	struct MethodInfo;

	struct TypeRef {
		Type type;
		TypeInfo& info;
	};

	struct FieldRef {
		Name name;
		FieldInfo& info;
	};

	struct MethodRef {
		Name name;
		MethodInfo& info;
	};

	struct TypeFieldRef {
		TypeRef type;
		FieldRef field;
	};

	struct TypeMethodRef {
		TypeRef type;
		MethodRef method;
	};
}
