#pragma once

#include "Util.h"

#include <UTemplate/TypeID.h>

#include <memory>
#include <cassert>

namespace Ubpa::UDRefl {
	using SharedBuffer = std::shared_ptr<void>;
	using SharedConstBuffer = std::shared_ptr<const void>;

	struct ResultDesc {
		TypeID typeID{ TypeID::of<void> };
		size_t size{ 0 };
		size_t alignment{ 0 };

		constexpr bool IsVoid() const noexcept {
			return typeID == TypeID::of<void>;
		}
	};

	struct InvokeResult {
		bool success{ false };
		TypeID resultID;
		Destructor destructor;

		template<typename T>
		T Move(void* result_buffer) {
			assert(result_buffer);

			if constexpr (!std::is_reference_v<T> && std::is_default_constructible_v<T>) {
				if (!success)
					return {};
			}
			else
				assert(success);

			assert(resultID = TypeID::of<T>);

			if constexpr (std::is_reference_v<T>) {
				using PtrT = std::add_pointer_t<std::remove_reference_t<T>>;
				assert(!destructor);
				return std::forward<T>(*buffer_as<PtrT>(result_buffer));
			}
			else {
				T rst = std::move(buffer_as<type_buffer_decay_t<T>>(result_buffer));
				if (destructor)
					destructor(result_buffer);
				return std::forward<T>(rst);
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
