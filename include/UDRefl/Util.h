#pragma once

#include <cstdint>
#include <type_traits>

namespace Ubpa::UDRefl {
	struct has_virtual_base_void {};
	template<typename Void, typename Obj>
	struct has_virtual_base_helper : std::true_type {};
	template<typename Obj>
	struct has_virtual_base_helper<
		std::void_t<decltype(reinterpret_cast<has_virtual_base_void has_virtual_base_void::*>(std::declval<has_virtual_base_void Obj::*>()))>,
		Obj> : std::false_type{};

	template<typename T>
	struct has_virtual_base : has_virtual_base_helper<void, T> {};
	template<typename T>
	constexpr bool has_virtual_base_v = has_virtual_base<T>::value;

	template<typename Obj, typename T>
	std::size_t field_offset(T Obj::* field_ptr) noexcept {
		static_assert(!std::is_function_v<T>);
		static_assert(!has_virtual_base_v<T>);
		return reinterpret_cast<std::size_t>(
			&(reinterpret_cast<Obj const volatile*>(nullptr)->*field_ptr)
		);
	}

	template<typename FieldPtr, FieldPtr fieldptr>
	struct field_offset_functor_impl;

	template<typename Obj, typename T, T Obj::* fieldptr>
	struct field_offset_functor_impl<T Obj::*, fieldptr> {
		static_assert(!std::is_function_v<T>);
		static constexpr auto get() noexcept {
			return [](const void* ptr) noexcept -> const void* {
				return &(reinterpret_cast<const Obj*>(ptr)->*fieldptr);
			};
		}
	};

	template<auto fieldptr>
	constexpr auto field_offset_functor() noexcept {
		return field_offset_functor_impl<decltype(fieldptr), fieldptr>::get();
	}

	// for non-virtual base
	template<typename Derived, typename Base>
	std::size_t base_offset() noexcept {
		static_assert(std::is_base_of_v<Base, Derived>);
		return reinterpret_cast<std::size_t>(
			static_cast<Base*>(reinterpret_cast<Derived*>(1))
		) - 1;
	}

	// for virtual base
	template<typename Derived, typename Base>
	constexpr auto base_offset_functor() noexcept {
		static_assert(std::is_base_of_v<Base, Derived>);
		return [](const void* obj) noexcept -> const void* {
			return static_cast<const Base*>(reinterpret_cast<const Derived*>(obj));
		};
	}

	constexpr void* forward_offset(void* ptr, std::size_t offset) noexcept {
		return (std::uint8_t*)ptr + offset;
	}

	constexpr const void* forward_offset(const void* ptr, std::size_t offset) noexcept {
		return forward_offset(const_cast<void*>(ptr), offset);
	}

	constexpr void* backward_offset(void* ptr, std::size_t offset) noexcept {
		return (std::uint8_t*)ptr - offset;
	}

	constexpr const void* backward_offset(const void* ptr, std::size_t offset) noexcept {
		return backward_offset(const_cast<void*>(ptr), offset);
	}
}
