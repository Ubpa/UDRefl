#pragma once

template<typename Enum> requires std::is_enum_v<Enum>
constexpr decltype(auto) Ubpa::UDRefl::enum_cast(Enum&& e) noexcept {
	using E = decltype(e);
	using T = std::underlying_type_t<std::remove_cvref_t<Enum>>;
	if constexpr (std::is_reference_v<E>) {
		using UnrefE = std::remove_reference_t<E>;
		if constexpr (std::is_lvalue_reference_v<E>) {
			if constexpr (std::is_const_v<UnrefE>)
				return static_cast<const T&>(e);
			else
				return static_cast<T&>(e);
		}
		else if constexpr (std::is_rvalue_reference_v<E>) {
			if constexpr (std::is_const_v<UnrefE>)
				return static_cast<const T&&>(e);
			else
				return static_cast<T&&>(e);
		}
		else {
			if constexpr (std::is_const_v<UnrefE>)
				return static_cast<const T>(e);
			else
				return static_cast<T>(e);
		}
	}

}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr bool Ubpa::UDRefl::enum_empty(const Enum& e) noexcept {
	using T = std::underlying_type_t<Enum>;
	return static_cast<T>(e);
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr bool Ubpa::UDRefl::enum_single(const Enum& e) noexcept {
	using T = std::underlying_type_t<Enum>;
	return (static_cast<T>(e) & (static_cast<T>(e) - 1)) == static_cast<T>(0);
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr bool Ubpa::UDRefl::enum_contain_any(const Enum& e, const Enum& flag) noexcept {
	using T = std::underlying_type_t<Enum>;
	return static_cast<T>(e) & static_cast<T>(flag);
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr bool Ubpa::UDRefl::enum_contain(const Enum& e, const Enum& flag) noexcept {
	using T = std::underlying_type_t<Enum>;
	const auto flag_T = static_cast<T>(flag);
	return (static_cast<T>(e) & flag_T) == flag_T;
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr Enum Ubpa::UDRefl::enum_combine(std::initializer_list<Enum> flags) noexcept {
	using T = std::underlying_type_t<Enum>;
	T rst = 0;
	for (const auto& flag : flags)
		rst |= static_cast<T>(flag);
	return static_cast<Enum>(rst);
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr Enum Ubpa::UDRefl::enum_remove(const Enum& e, const Enum& flag) noexcept {
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<T>(e) & (~static_cast<T>(flag)));
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr Enum Ubpa::UDRefl::enum_within(const Enum& e, const Enum& flag) noexcept {
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<T>(e) & (static_cast<T>(flag)));
}

constexpr bool Ubpa::UDRefl::is_ref_compatible(Type lhs, Type rhs) noexcept {
	if (lhs == rhs)
		return true;

	if (lhs.IsLValueReference()) { // &{T} | &{const{T}}
		const auto unref_lhs = lhs.Name_RemoveLValueReference(); // T | const{T}
		if (type_name_is_const(unref_lhs)) { // &{const{T}}
			if (unref_lhs == rhs.Name_RemoveRValueReference())
				return true; // &{const{T}} <- &&{const{T}} | const{T}

			const auto raw_lhs = type_name_remove_const(unref_lhs); // T

			if (rhs.Is(raw_lhs) || raw_lhs == rhs.Name_RemoveReference())
				return true; // &{const{T}} <- T | &{T} | &&{T}
		}
	}
	else if (lhs.IsRValueReference()) { // &&{T} | &&{const{T}}
		const auto unref_lhs = lhs.Name_RemoveRValueReference(); // T | const{T}

		if (type_name_is_const(unref_lhs)) { // &&{const{T}}
			if (rhs.Is(unref_lhs))
				return true; // &&{const{T}} <- const{T}

			const auto raw_lhs = type_name_remove_const(unref_lhs); // T

			if (rhs.Is(raw_lhs))
				return true; // &&{const{T}} <- T

			if (raw_lhs == rhs.Name_RemoveRValueReference())
				return true; // &&{const{T}} <- &&{T}
		}
		else {
			if (rhs.Is(unref_lhs))
				return true; // &&{T} <- T
		}
	}
	else { // T
		if (lhs.Is(rhs.Name_RemoveRValueReference()))
			return true; // T <- &&{T}
	}

	return false;
}

constexpr bool Ubpa::UDRefl::is_pointer_array_compatible(std::string_view lhs, std::string_view rhs) noexcept {
	if (type_name_is_reference(lhs)) {
		lhs = type_name_remove_reference(lhs);
		if (!type_name_is_const(lhs))
			return false;
		lhs = type_name_remove_const(lhs);
	}
	rhs = type_name_remove_cvref(rhs);

	if (lhs == rhs)
		return true;

	std::string_view lhs_ele;
	if (type_name_is_pointer(lhs))
		lhs_ele = type_name_remove_pointer(lhs);
	else if (type_name_is_unbounded_array(lhs))
		lhs_ele = type_name_remove_extent(lhs);
	else
		return false;

	std::string_view rhs_ele;
	if (type_name_is_pointer(rhs))
		rhs_ele = type_name_remove_pointer(rhs);
	else if (type_name_is_array(rhs))
		rhs_ele = type_name_remove_extent(rhs);
	else
		return false;

	return lhs_ele == rhs_ele || type_name_remove_const(lhs_ele) == rhs_ele;
}

template<typename T>
struct Ubpa::UDRefl::get_container_size_type : std::type_identity<typename T::size_type> {};
template<typename T>
struct Ubpa::UDRefl::get_container_size_type<T&> : Ubpa::UDRefl::get_container_size_type<T> {};
template<typename T>
struct Ubpa::UDRefl::get_container_size_type<T&&> : Ubpa::UDRefl::get_container_size_type<T> {};
template<typename T, std::size_t N>
struct Ubpa::UDRefl::get_container_size_type<T[N]> : std::type_identity<std::size_t> {};
template<typename T>
struct Ubpa::UDRefl::get_container_size_type<T[]> : std::type_identity<std::size_t> {};
