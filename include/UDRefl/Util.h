#pragma once

#include <UTemplate/Func.h>

#include <cstdint>
#include <functional>

namespace Ubpa::UDRefl {
	using Offsetor = std::function<const void* (const void*)>;
	using Destructor = std::function<void(const void*)>;
	using FreeFunc = std::function<void(void*)>;
	using DeleteFunc = std::function<void(void*)>; // Destructor + FreeFunc

	template<typename Obj, typename T>
	std::size_t field_forward_offset_value(T Obj::* field_ptr) noexcept {
		static_assert(!std::is_function_v<T>);
		static_assert(!has_virtual_base_v<Obj>);
		return reinterpret_cast<std::size_t>(&(reinterpret_cast<Obj const volatile*>(nullptr)->*field_ptr));
	}

	template<auto fieldptr>
	struct field_offsetor_impl;

	template<typename Obj, typename T, T Obj::* fieldptr>
	struct field_offsetor_impl<fieldptr> {
		static_assert(!std::is_function_v<T>);
		static constexpr auto get() noexcept {
			return [](const void* ptr) noexcept -> const void* {
				return &(reinterpret_cast<const Obj*>(ptr)->*fieldptr);
			};
		}
	};

	template<auto fieldptr>
	constexpr auto field_offsetor() noexcept {
		return field_offsetor_impl<fieldptr>::get();
	}

	// result size of field_offsetor(fieldptr) > result size of field_offsetor<fieldptr> 
	template<typename T, typename Obj>
	constexpr auto field_offsetor(T Obj::* fieldptr) noexcept {
		static_assert(!std::is_function_v<T>);
		return [fieldptr](const void* ptr) noexcept -> const void* {
			return &(reinterpret_cast<const Obj*>(ptr)->*fieldptr);
		};
	}

	struct InheritCastFunctions {
		Offsetor static_derived_to_base;
		Offsetor static_base_to_derived;
		Offsetor dynamic_base_to_derived;
	};

	template<typename From, typename To>
	constexpr auto static_cast_functor() noexcept {
		static_assert(!is_virtual_base_of_v<From, To>);
		return [](const void* obj) noexcept -> const void* {
			return static_cast<const To*>(reinterpret_cast<const From*>(obj));
		};
	}

	template<typename Base, typename Derived>
	constexpr auto dynamic_cast_function() noexcept {
		static_assert(std::is_base_of_v<Base, Derived>);
		if constexpr (std::is_polymorphic_v<Base>) {
			return [](const void* obj) noexcept -> const void* {
				return dynamic_cast<const Derived*>(reinterpret_cast<const Base*>(obj));
			};
		}
		else
			return static_cast_functor<Base, Derived>();
	}

	// polymorphic: dynamic_cast
	// virtual    : no static_cast (Base -> Derived)
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
	constexpr Destructor destructor() noexcept {
		if constexpr (std::is_fundamental_v<T> || std::is_compound_v<T>)
			return {};
		else {
			static_assert(std::is_destructible_v<T>);
			if constexpr (!std::is_trivially_destructible_v<T>) {
				return [](const void* ptr) {
					reinterpret_cast<const T*>(ptr)->~T();
				};
			}
			else
				return {};
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

	template<typename T>
	constexpr T& buffer_get(void* buffer, std::size_t offset) noexcept {
		auto ptr = forward_offset(buffer, offset);
		return *reinterpret_cast<T*>(ptr);
	}

	template<typename T>
	constexpr const T& buffer_get(const void* buffer, std::size_t offset) noexcept {
		return buffer_get<T>(const_cast<void*>(buffer), offset);
	}

	template<typename T>
	constexpr T& buffer_as(void* buffer) noexcept {
		return buffer_get<T>(buffer, 0);
	}

	template<typename T>
	constexpr const T& buffer_as(const void* buffer) noexcept {
		return buffer_get<T>(buffer, 0);
	}

	// [func_ptr]
	// - Func Obj::* : Func isn't && (const && is ok)
	// - Func*
	// [result]
	// - type : ({const?} void* obj, void* result_buffer, void* args_buffer) -> Destructor
	// - size : 1
	template<auto func_ptr>
	constexpr auto wrap_member_function() noexcept;

	// [func_ptr]
	// - Func*
	// [result]
	// - type : (void* result_buffer, void* args_buffer) -> Destructor
	// - size : 1
	template<auto func_ptr>
	constexpr auto wrap_static_function() noexcept;

	// static dispatch to
	// - wrap_member_function
	// - wrap_static_function
	template<auto func_ptr>
	constexpr auto wrap_function() noexcept;

	// Func: Ret(const? volatile? Object&, Args...)
	// [result]
	// - type : ({const?} void* obj, void* result_buffer, void* args_buffer) -> Destructor
	// - size : sizeof(Func)
	template<typename Func>
	constexpr auto wrap_member_function(Func&& func) noexcept;

	// Func: Ret(Args...)
	// [result]
	// - type : (void* result_buffer, void* args_buffer) -> Destructor
	// - size : sizeof(Func)
	template<typename Func>
	constexpr auto wrap_static_function(Func&& func) noexcept;

	//
	// traits
	///////////

	template<typename T>
	struct is_iterator;
	template<typename T>
	constexpr bool is_iterator_v = is_iterator<T>::value;

	template<typename T>
	using operator_plus = decltype(+std::declval<const T&>());
	template<typename T>
	using operator_minus = std::enable_if_t<!std::is_unsigned_v<T>, decltype(-std::declval<const T&>())>;

	template<typename T, typename U = const T&>
	using operator_add = decltype(std::declval<const T&>() + std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_sub = decltype(std::declval<const T&>() - std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_mul = decltype(std::declval<const T&>()* std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_div = decltype(std::declval<const T&>() / std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_mod = decltype(std::declval<const T&>() % std::declval<U>());

	template<typename T>
	using operator_bnot = decltype(~std::declval<const T&>());
	template<typename T, typename U = const T&>
	using operator_band = decltype(std::declval<const T&>()& std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_bor = decltype(std::declval<const T&>() | std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_bxor = decltype(std::declval<const T&>() ^ std::declval<U>());
	template<typename T, typename U = std::ostream&>
	using operator_lshift = decltype(std::declval<U>() << std::declval<T>());
	template<typename T, typename U = std::istream&>
	using operator_rshift = decltype(std::declval<U>() >> std::declval<T>());

	template<typename T>
	using operator_pre_inc = decltype(++std::declval<T&>());
	template<typename T>
	using operator_post_inc = decltype(std::declval<T&>()++);
	template<typename T>
	using operator_pre_dec = decltype(--std::declval<T&>());
	template<typename T>
	using operator_post_dec = decltype(std::declval<T&>()--);

	template<typename T, typename U = const T&>
	using operator_assign = decltype(std::declval<T&>() = std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_assign_add = decltype(std::declval<T&>() += std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_assign_sub = decltype(std::declval<T&>() -= std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_assign_mul = decltype(std::declval<T&>() *= std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_assign_div = decltype(std::declval<T&>() /= std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_assign_mod = decltype(std::declval<T&>() %= std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_assign_band = decltype(std::declval<T&>() &= std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_assign_bor = decltype(std::declval<T&>() |= std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_assign_bxor = decltype(std::declval<T&>() ^= std::declval<U>());
	template<typename T, typename U = std::size_t>
	using operator_assign_lshift = decltype(std::declval<T&>() <<= std::declval<U>());
	template<typename T, typename U = std::size_t>
	using operator_assign_rshift = decltype(std::declval<T&>() >>= std::declval<U>());

	template<typename T, typename U = const T&>
	using operator_eq = decltype(std::declval<const T&>() == std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_neq = decltype(std::declval<const T&>() != std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_lt = decltype(std::declval<const T&>() < std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_le = decltype(std::declval<const T&>() <= std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_gt = decltype(std::declval<const T&>() > std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_ge = decltype(std::declval<const T&>() >= std::declval<U>());

	template<typename T, typename U = const T&>
	using operator_and = decltype(std::declval<const T&>() && std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_or = decltype(std::declval<const T&>() || std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_not = decltype(!std::declval<const T&>());

	template<typename T, typename U = std::size_t>
	using operator_subscript = decltype(std::declval<T&>()[U]);
	template<typename T, typename U = std::size_t>
	using operator_subscript_const = decltype(std::declval<const T&>()[U]);
	template<typename T>
	using operator_deref = decltype(*std::declval<T&>());
	template<typename T>
	using operator_deref_const = decltype(*std::declval<const T&>());
	template<typename T>
	using operator_ref = decltype(&std::declval<T&>());
	template<typename T>
	using operator_ref_const = decltype(&std::declval<const T&>());
	template<typename T>
	using operator_member = decltype(std::declval<T&>().operator->());
	template<typename T>
	using operator_member_const = decltype(std::declval<const T&>().operator->());
	template<typename T, typename U>
	using operator_member_of_pointer = decltype(std::declval<T&>().operator->*(std::declval<U>()));
	template<typename T, typename U>
	using operator_member_of_pointer_const = decltype(std::declval<const T&>().operator->*(std::declval<U>()));
}

#include "details/Util.inl"
