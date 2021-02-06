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

		None     = 0b000,
		All      = 0b111
	};
	UBPA_UDREFL_ENUM_BOOL_OPERATOR_DEFINE(FuncFlag)

	enum class FieldFlag {
		Basic         = 0b00001,
		Virtual       = 0b00010,
		Static        = 0b00100,
		DynamicShared = 0b01000,
		DynamicBuffer = 0b10000,

		None          = 0b00000,
		Owned         = 0b00011,
		Unowned       = 0b11100,
		All           = 0b11111
	};
	UBPA_UDREFL_ENUM_BOOL_OPERATOR_DEFINE(FieldFlag)

	using SharedBuffer = std::shared_ptr<void>;
	class ObjectView;
	class SharedObject;

	template<typename T>
	struct IsObjectOrView {
	private:
		using U = std::remove_cvref_t<T>;
	public:
		static constexpr bool value =
			std::is_same_v<U, ObjectView>
			|| std::is_same_v<U, SharedObject>;
	};
	template<typename T> constexpr bool IsObjectOrView_v = IsObjectOrView<T>::value;
	template<typename T> concept NonObjectAndView = !IsObjectOrView_v<T>;

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
