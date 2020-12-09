#pragma once

#include "Registry.h"

namespace Ubpa::UDRefl {
	inline ObjectPtr ArgsView::At(size_t idx) const noexcept {
		assert(idx < paramList.GetParameters().size());
		return {
			paramList.GetParameters()[idx].typeID,
			forward_offset(buffer, paramList.GetOffsets()[idx])
		};
	}

	template<typename T>
	static MethodPtr MethodPtr::GenerateDefaultConstructor() noexcept {
		return { static_cast<ObjectVariableFunction*>(
			[](void* obj, ArgsView, void*) -> Destructor* {
				assert(obj);
				new(obj)T;
				return nullptr;
			}
		) };
	}

	template<typename T>
	static MethodPtr MethodPtr::GenerateDestructor() noexcept {
		return { static_cast<ObjectConstFunction*>(
			[](const void* obj, ArgsView, void*) -> Destructor* {
				assert(obj);
				reinterpret_cast<const T*>(obj)->~T();
				return nullptr;
			}
		) };
	}

	template<typename T>
	T InvokeResult::Move(void* result_buffer) {
		assert(result_buffer);

		if constexpr (!std::is_lvalue_reference_v<T> && std::is_default_constructible_v<std::remove_reference_t<T>>) {
			if (!success)
				return std::forward<T>(T{});
		}
		else
			assert(success);

		assert(resultID = TypeRegistry::DirectGetID<T>());

		if constexpr (std::is_lvalue_reference_v<T>) {
			using PtrT = std::add_pointer_t<std::remove_reference_t<T>>;
			assert(destructor == nullptr);
			return *buffer_as<PtrT>(result_buffer);
		}
		else {
			T rst = std::move(buffer_as<T>(result_buffer));
			if (destructor)
				destructor(result_buffer);
			return rst;
		}
	}
}
