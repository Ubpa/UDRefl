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
		return reinterpret_cast<std::size_t>(&(reinterpret_cast<const Obj*>(0)->*field_ptr));
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
	InheritCastFunctions inherit_cast_functions() {
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
	Destructor destructor() {
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

	//
	// Wrap
	/////////
	//
	// - if result is reference, function will store a pointer in the result buffer
	// - return type can't be const/volatile
	// 

	// pointer const array type (pointer is const, and pointer to non - const / referenced object)
	using ArgsBuffer = void* const*;

	// [func_ptr]
	// - Func Obj::* : Func isn't && (const && is ok)
	// - Func*
	// [result]
	// - type : ({const?} void* obj, void* result_buffer, ArgsBuffer args_buffer) -> Destructor
	// - size : 1
	template<auto func_ptr>
	constexpr auto wrap_member_function() noexcept;

	// [func_ptr]
	// - Func*
	// [result]
	// - type : (void* result_buffer, ArgsBuffer args_buffer) -> Destructor
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
	// - type : ({const?} void* obj, void* result_buffer, ArgsBuffer args_buffer) -> Destructor
	// - size : sizeof(Func)
	template<typename Func>
	constexpr auto wrap_member_function(Func&& func) noexcept;

	// Func: Ret(Args...)
	// [result]
	// - type : (void* result_buffer, ArgsBuffer args_buffer) -> Destructor
	// - size : sizeof(Func)
	template<typename Func>
	constexpr auto wrap_static_function(Func&& func) noexcept;

	//
	// Traits
	///////////

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
	using operator_div = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<const T&>() / std::declval<U>())>;
	template<typename T, typename U = const T&>
	using operator_mod = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<const T&>() % std::declval<U>())>;

	template<typename T>
	using operator_bnot = std::enable_if_t<!std::is_same_v<T, bool>, decltype(~std::declval<const T&>())>;
	template<typename T, typename U = const T&>
	using operator_band = decltype(std::declval<const T&>()& std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_bor = decltype(std::declval<const T&>() | std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_bxor = decltype(std::declval<const T&>() ^ std::declval<U>());
	template<typename T, typename U>
	using operator_lshift = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<U>() >> std::declval<T&>())>;
	template<typename T, typename U>
	using operator_rshift = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<U>() << std::declval<const T&>())>;

	template<typename T>
	using operator_pre_inc = std::enable_if_t<!std::is_same_v<T, bool>, decltype(++std::declval<T&>())>;
	template<typename T>
	using operator_post_inc = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>()++)>;
	template<typename T>
	using operator_pre_dec = std::enable_if_t<!std::is_same_v<T, bool>, decltype(--std::declval<T&>())>;
	template<typename T>
	using operator_post_dec = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>()--)>;

	template<typename T>
	using operator_assign_copy = std::enable_if_t<std::is_copy_assignable_v<T>, decltype(std::declval<T&>() = std::declval<const T&>())> ;
	template<typename T>
	using operator_assign_move = std::enable_if_t<std::is_move_assignable_v<T>, decltype(std::declval<T&>() = std::declval<T&&>())>;
	template<typename T, typename U = const T&>
	using operator_assign_add = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>() += std::declval<U>())>;
	template<typename T, typename U = const T&>
	using operator_assign_sub = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>() -= std::declval<U>())>;
	template<typename T, typename U = const T&>
	using operator_assign_mul = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>() *= std::declval<U>())>;
	template<typename T, typename U = const T&>
	using operator_assign_div = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>() /= std::declval<U>())>;
	template<typename T, typename U = const T&>
	using operator_assign_mod = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>() %= std::declval<U>())>;
	template<typename T, typename U = const T&>
	using operator_assign_band = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>() &= std::declval<U>())>;
	template<typename T, typename U = const T&>
	using operator_assign_bor = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>() |= std::declval<U>())>;
	template<typename T, typename U = const T&>
	using operator_assign_bxor = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>() ^= std::declval<U>())>;
	template<typename T, typename U = std::size_t>
	using operator_assign_lshift = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>() <<= std::declval<U>())>;
	template<typename T, typename U = std::size_t>
	using operator_assign_rshift = std::enable_if_t<!std::is_same_v<T, bool>, decltype(std::declval<T&>() >>= std::declval<U>())>;

	template<typename T, typename U = const T&>
	using operator_eq = decltype(std::declval<const T&>() == std::declval<U>());
	template<typename T, typename U = const T&>
	using operator_ne = decltype(std::declval<const T&>() != std::declval<U>());
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
	using operator_subscript = decltype(std::declval<T&>()[std::declval<U>()]);
	template<typename T, typename U = std::size_t>
	using operator_subscript_const = decltype(std::declval<const T&>()[std::declval<U>()]);
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

	// iterator

	template<typename T>
	struct is_iterator;
	template<typename T>
	constexpr bool is_iterator_v = is_iterator<T>::value;

	template<typename T, typename U = std::size_t>
	using iterator_add = decltype(std::declval<const T&>() + std::declval<U>);
	template<typename T, typename U = std::size_t>
	using iterator_sub = decltype(std::declval<const T&>() - std::declval<U>);

	// container

	template<typename T, typename U = typename T::size_type, typename V = const typename T::value_type&>
	using container_assign = decltype(std::declval<T&>().assign(std::declval<U>(), std::declval<V>()));

	// - iterator

	template<typename T>
	using container_begin = decltype(std::declval<T&>().begin());
	template<typename T>
	using container_begin_const = decltype(std::declval<const T&>().begin());
	template<typename T>
	using container_cbegin = decltype(std::declval<const T&>().cbegin());

	template<typename T>
	using container_end = decltype(std::declval<T&>().end());
	template<typename T>
	using container_end_const = decltype(std::declval<const T&>().end());
	template<typename T>
	using container_cend = decltype(std::declval<const T&>().cend());

	template<typename T>
	using container_rbegin = decltype(std::declval<T&>().rbegin());
	template<typename T>
	using container_rbegin_const = decltype(std::declval<const T&>().rbegin());
	template<typename T>
	using container_crbegin = decltype(std::declval<const T&>().crbegin());

	template<typename T>
	using container_rend = decltype(std::declval<T&>().rend());
	template<typename T>
	using container_rend_const = decltype(std::declval<const T&>().rend());
	template<typename T>
	using container_crend = decltype(std::declval<const T&>().crend());

	// - element access

	template<typename T, typename U = typename T::size_type>
	using container_at = decltype(std::declval<T&>().at(std::declval<U>()));
	template<typename T, typename U = typename T::size_type>
	using container_at_const = decltype(std::declval<const T&>().at(std::declval<U>()));

	template<typename T, typename U = const typename T::key_type&>
	using container_at_key = decltype(std::declval<T&>().at(std::declval<U>()));
	template<typename T, typename U = const typename T::key_type&>
	using container_at_key_const = decltype(std::declval<const T&>().at(std::declval<U>()));

	template<typename T, typename U = const typename T::key_type&>
	using container_subscript_key_0 = decltype(std::declval<T&>()[std::declval<U>()]);
	template<typename T, typename U = typename T::key_type&&>
	using container_subscript_key_1 = decltype(std::declval<T&>()[std::declval<U>()]);

	template<typename T, typename U = typename T::size_type>
	using container_subscript = decltype(std::declval<T&>()[std::declval<U>()]);
	template<typename T, typename U = typename T::size_type>
	using container_subscript_const = decltype(std::declval<const T&>()[std::declval<U>()]);

	template<typename T>
	using container_data = decltype(std::declval<T&>().data());
	template<typename T>
	using container_data_const = decltype(std::declval<const T&>().data());

	template<typename T>
	using container_front = decltype(std::declval<T&>().front());
	template<typename T>
	using container_front_const = decltype(std::declval<const T&>().front());

	template<typename T>
	using container_back = decltype(std::declval<T&>().back());
	template<typename T>
	using container_back_const = decltype(std::declval<const T&>().back());

	// - capacity

	template<typename T>
	using container_empty = decltype(std::declval<const T&>().empty());

	template<typename T>
	using container_size = decltype(std::declval<const T&>().size());

	template<typename T>
	using container_max_size = decltype(std::declval<const T&>().max_size());

	template<typename T, typename U = typename T::size_type>
	using container_resize_0 = decltype(std::declval<T&>().resize(std::declval<U>()));

	template<typename T, typename U = typename T::size_type, typename V = const typename T::value_type&>
	using container_resize_1 = decltype(std::declval<T&>().resize(std::declval<U>(), std::declval<V>()));

	template<typename T>
	using container_capacity = decltype(std::declval<const T&>().capacity());

	template<typename T>
	using container_bucket_count = decltype(std::declval<const T&>().bucket_count());

	template<typename T, typename U = typename T::size_type>
	using container_reserve = decltype(std::declval<T&>().reserve(std::declval<U>()));

	template<typename T>
	using container_shrink_to_fit = decltype(std::declval<T&>().shrink_to_fit());

	// - modifiers

	template<typename T>
	using container_clear = decltype(std::declval<T&>().clear());

	template<typename T, typename U = typename T::const_iterator, typename V = const typename T::value_type&>
	using container_insert_0 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>()));
	template<typename T, typename U = typename T::const_iterator, typename V = typename T::value_type&&>
	using container_insert_1 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>()));
	template<typename T, typename U = typename T::const_iterator, typename V = typename T::size_type, typename W = const typename T::value_type&>
	using container_insert_2 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>(), std::declval<W>()));
	template<typename T, typename U = typename T::node_type&&>
	using container_insert_3 = decltype(std::declval<T&>().insert(std::declval<U>()));
	template<typename T, typename U = typename T::const_iterator, typename V = typename T::node_type&&>
	using container_insert_4 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>()));

	template<typename T, typename U = const typename T::key_type&, typename V = const typename T::mapped_type&>
	using container_insert_or_assign_0 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>()));
	template<typename T, typename U = const typename T::key_type&, typename V = const typename T::mapped_type&&>
	using container_insert_or_assign_1 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>()));
	template<typename T, typename U = typename T::key_type&&, typename V = const typename T::mapped_type&>
	using container_insert_or_assign_2 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>()));
	template<typename T, typename U = typename T::key_type&&, typename V = const typename T::mapped_type&&>
	using container_insert_or_assign_3 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>()));
	template<typename T, typename U = typename T::const_iterator, typename V = const typename T::key_type&, typename W = const typename T::mapped_type&>
	using container_insert_or_assign_4 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>(), std::declval<W>()));
	template<typename T, typename U = typename T::const_iterator, typename V = const typename T::key_type&, typename W = const typename T::mapped_type&&>
	using container_insert_or_assign_5 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>(), std::declval<W>()));
	template<typename T, typename U = typename T::const_iterator, typename V = typename T::key_type&&, typename W = const typename T::mapped_type&>
	using container_insert_or_assign_6 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>(), std::declval<W>()));
	template<typename T, typename U = typename T::const_iterator, typename V = typename T::key_type&&, typename W = const typename T::mapped_type&&>
	using container_insert_or_assign_7 = decltype(std::declval<T&>().insert(std::declval<U>(), std::declval<V>(), std::declval<W>()));

	template<typename T, typename U = typename T::iterator>
	using container_erase_0 = decltype(std::declval<T&>().erase(std::declval<U>()));
	template<typename T, typename U = typename T::const_iterator>
	using container_erase_1 = decltype(std::declval<T&>().erase(std::declval<U>()));
	template<typename T, typename U = typename T::iterator>
	using container_erase_2 = decltype(std::declval<T&>().erase(std::declval<U>(), std::declval<U>()));
	template<typename T, typename U = typename T::const_iterator>
	using container_erase_3 = decltype(std::declval<T&>().erase(std::declval<U>(), std::declval<U>()));
	template<typename T, typename U = const typename T::key_type&>
	using container_erase_4 = decltype(std::declval<T&>().erase(std::declval<U>()));

	template<typename T, typename U = const typename T::value_type&>
	using container_push_front_0 = decltype(std::declval<T&>().push_front(std::declval<U>()));
	template<typename T, typename U = typename T::value_type&&>
	using container_push_front_1 = decltype(std::declval<T&>().push_front(std::declval<U>()));

	template<typename T>
	using container_pop_front = decltype(std::declval<T&>().pop_front());

	template<typename T, typename U = const typename T::value_type&>
	using container_push_back_0 = decltype(std::declval<T&>().push_back(std::declval<U>()));
	template<typename T, typename U = typename T::value_type&&>
	using container_push_back_1 = decltype(std::declval<T&>().push_back(std::declval<U>()));

	template<typename T>
	using container_pop_back = decltype(std::declval<T&>().pop_back());

	template<typename T>
	using container_swap = decltype(std::swap(std::declval<T&>(), std::declval<T&>()));

	template<typename T, typename U = const T&>
	using container_merge_0 = decltype(std::declval<T&>().merge(std::declval<U>()));
	template<typename T, typename U = T&&>
	using container_merge_1 = decltype(std::declval<T&>().merge(std::declval<U>()));

	template<typename T, typename U = typename T::const_iterator>
	using container_extract_0 = decltype(std::declval<T&>().extract(std::declval<U>()));
	template<typename T, typename U = const typename T::key_type&>
	using container_extract_1 = decltype(std::declval<T&>().extract(std::declval<U>()));

	// - lookup

	template<typename T, typename U = const typename T::key_type&>
	using container_count = decltype(std::declval<const T&>().count(std::declval<U>()));

	template<typename T, typename U = const typename T::key_type&>
	using container_find = decltype(std::declval<T&>().count(std::declval<U>()));
	template<typename T, typename U = const typename T::key_type&>
	using container_find_const = decltype(std::declval<const T&>().count(std::declval<U>()));
	// contains (C++20)
	template<typename T, typename U = const typename T::key_type&>
	using container_lower_bound = decltype(std::declval<T&>().lower_bound(std::declval<U>()));
	template<typename T, typename U = const typename T::key_type&>
	using container_lower_bound_const = decltype(std::declval<const T&>().lower_bound(std::declval<U>()));
	template<typename T, typename U = const typename T::key_type&>
	using container_upper_bound = decltype(std::declval<T&>().lower_bound(std::declval<U>()));
	template<typename T, typename U = const typename T::key_type&>
	using container_upper_bound_const = decltype(std::declval<const T&>().lower_bound(std::declval<U>()));
	template<typename T, typename U = const typename T::key_type&>
	using container_equal_range = decltype(std::declval<T&>().equal_range(std::declval<U>()));
	template<typename T, typename U = const typename T::key_type&>
	using container_equal_range_const = decltype(std::declval<const T&>().equal_range(std::declval<U>()));

	// - observers

	template<typename T>
	using container_key_comp = decltype(std::declval<const T&>().key_comp());
	template<typename T>
	using container_value_comp = decltype(std::declval<const T&>().value_comp());
	template<typename T>
	using container_hash_function = decltype(std::declval<const T&>().hash_function());
	template<typename T>
	using container_key_eq = decltype(std::declval<const T&>().key_eq());
	template<typename T>
	using container_get_allocator = decltype(std::declval<const T&>().get_allocator());

	// - list operations (TODO)
}

#include "details/Util.inl"
