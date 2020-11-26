#pragma once

#include <cstdint>
#include <type_traits>

namespace Ubpa::UDRefl {
	template<typename Obj, typename T>
	std::size_t field_offset(T Obj::* field_ptr) noexcept {
		static_assert(!std::is_function_v<T>);
		return reinterpret_cast<std::size_t>(
			&(reinterpret_cast<Obj const volatile*>(nullptr)->*field_ptr)
		);
	}

	template<typename Derived, typename Base>
	std::size_t base_offset() noexcept {
		static_assert(std::is_base_of_v<Base, Derived>);
		return reinterpret_cast<std::size_t>(
			static_cast<Base*>(reinterpret_cast<Derived*>(1))
		) - 1;
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
