#pragma once

#include <cstdint>
#include <type_traits>

namespace Ubpa::UDRefl {
	using OffsetFunction = const void* (const void*) noexcept;
	using Destructor = void(void*);

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

	template<typename Void, typename Base, typename Derived>
	struct is_virtual_base_of_helper : std::is_base_of<Base, Derived> {};
	template<typename Base, typename Derived>
	struct is_virtual_base_of_helper<
		std::void_t<decltype(static_cast<Derived*>(std::declval<Base*>()))>,
		Base, Derived> : std::false_type {};

	template<typename Base, typename Derived>
	struct is_virtual_base_of : is_virtual_base_of_helper<void, Base, Derived> {};
	template<typename Base, typename Derived>
	constexpr bool is_virtual_base_of_v = is_virtual_base_of<Base, Derived>::value;

	template<typename Obj, typename T>
	std::size_t field_offset(T Obj::* field_ptr) noexcept {
		static_assert(!std::is_function_v<T>);
		static_assert(!has_virtual_base_v<T>);
		return reinterpret_cast<std::size_t>(
			&(reinterpret_cast<Obj const volatile*>(nullptr)->*field_ptr)
		);
	}

	template<typename FieldPtr, FieldPtr fieldptr>
	struct field_offset_function_impl;

	template<typename Obj, typename T, T Obj::* fieldptr>
	struct field_offset_function_impl<T Obj::*, fieldptr> {
		static_assert(!std::is_function_v<T>);
		static constexpr OffsetFunction* get() noexcept {
			return [](const void* ptr) noexcept -> const void* {
				return &(reinterpret_cast<const Obj*>(ptr)->*fieldptr);
			};
		}
	};

	template<auto fieldptr>
	constexpr OffsetFunction* field_offset_function() noexcept {
		return field_offset_function_impl<decltype(fieldptr), fieldptr>::get();
	}

	struct InheritCastFunctions {
		OffsetFunction* static_derived_to_base{ nullptr };
		OffsetFunction* static_base_to_derived{ nullptr };
		OffsetFunction* dynamic_base_to_derived{ nullptr };
	};

	template<typename From, typename To>
	constexpr OffsetFunction* static_cast_functor() noexcept {
		static_assert(!is_virtual_base_of_v<From, To>);
		return [](const void* obj) noexcept -> const void* {
			return static_cast<const To*>(reinterpret_cast<const From*>(obj));
		};
	}

	template<typename Base, typename Derived>
	constexpr OffsetFunction* dynamic_cast_function() noexcept {
		static_assert(std::is_base_of_v<Base, Derived>);
		if constexpr (std::is_polymorphic_v<Base>) {
			return [](const void* obj) noexcept -> const void* {
				return dynamic_cast<const Derived*>(reinterpret_cast<const Base*>(obj));
			};
		}
		else
			return static_cast_functor<Base, Derived>();
	}

	template<typename Derived, typename Base>
	constexpr InheritCastFunctions inherit_cast_functions() noexcept {
		static_assert(std::is_base_of_v<Base, Derived>);
		if constexpr (std::is_polymorphic_v<Derived>) {
			if constexpr (is_virtual_base_of_v<Base, Derived>) {
				return {
					static_cast_functor<Derived, Base>(),
					nullptr,
					dynamic_cast_function<Base, Derived>()
				};
			}
			else {
				return {
					static_cast_functor<Derived, Base>(),
					static_cast_functor<Base, Derived>(),
					dynamic_cast_function<Base, Derived>()
				};
			}
		}
		else{
			if constexpr (is_virtual_base_of_v<Base, Derived>) {
				return {
					static_cast_functor<Derived, Base>(),
					nullptr,
					nullptr
				};
			}
			else {
				return {
					static_cast_functor<Derived, Base>(),
					static_cast_functor<Base, Derived>(),
					nullptr
				};
			}
		}
	}

	template<typename T>
	constexpr Destructor* destructor() noexcept {
		if constexpr (std::is_fundamental_v<T> || std::is_compound_v<T>)
			return nullptr;
		else {
			static_assert(std::is_destructible_v<T>);
			if constexpr (!std::is_trivially_destructible_v<T>) {
				return [](void* ptr) {
					reinterpret_cast<T*>(ptr)->~T();
				};
			}
			else
				return nullptr;
		}
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
