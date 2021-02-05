#pragma once

#include "Util.h"

#include <UTemplate/TypeID.h>

#include <memory>
#include <cassert>

namespace Ubpa::UDRefl {
	using SharedBuffer = std::shared_ptr<void>;

	enum class ConstReferenceMode {
		None       = 0b000,
		Left       = 0b001,
		Right      = 0b010,
		Const      = 0b100,
		ConstLeft  = 0b101,
		ConstRight = 0b110,
	};

	struct ResultDesc {
		TypeID typeID{ TypeID_of<void> };
		size_t size{ 0 };
		size_t alignment{ 1 };

		constexpr bool IsVoid() const noexcept {
			return typeID == TypeID_of<void>;
		}
	};

	struct InvokeResult {
		bool success{ false };
		TypeID resultID;
		Destructor destructor;

		template<typename T>
		T Move(void* result_buffer) noexcept(std::is_reference_v<T> || std::is_nothrow_destructible_v<T> && std::is_nothrow_move_constructible_v<T>) {
			static_assert(!std::is_void_v<T>);

			assert(result_buffer);

			if constexpr (!std::is_reference_v<T> && std::is_default_constructible_v<T>) {
				if (!success || resultID != TypeID_of<T>)
					return {};
			}
			else
				assert(success && resultID == TypeID_of<T>);

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

		constexpr bool IsVoid() const noexcept {
			return resultID.Is<void>();
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
		TypeID ID;
		TypeInfo& info;
	};

	struct FieldRef {
		StrID ID;
		FieldInfo& info;
	};

	struct MethodRef {
		StrID ID;
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
