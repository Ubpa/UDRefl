#pragma once

#include <UTemplate/Type.h>

#include <string>
#include <unordered_map>
#include <memory_resource>

#ifndef NDEBUG
#include <unordered_set>
#endif // !NDEBUG

namespace Ubpa::UDRefl {
	// name must end with '\0'
	template<typename T>
	class IDRegistry {
	public:
		IDRegistry();

		void RegisterUnmanaged(T ID, std::string_view name);
		T RegisterUnmanaged(std::string_view name);
		void Register(T ID, std::string_view name);
		T Register(std::string_view name);

		bool IsRegistered(T ID) const;
		std::string_view Nameof(T ID) const;

		void UnregisterUnmanaged(T ID);
		void Clear() noexcept;

	protected:
		std::pmr::polymorphic_allocator<char> get_allocator() { return &resource; }

	private:
		std::pmr::monotonic_buffer_resource resource;
		std::pmr::unordered_map<T, std::string_view> id2name;

#ifndef NDEBUG
	public:
		bool IsUnmanaged(T ID) const;
		void ClearUnmanaged() noexcept;
	private:
		std::pmr::unordered_set<T> unmanagedIDs;
#endif // NDEBUG
	};

	class NameIDRegistry : public IDRegistry<NameID> {
	public:
		struct Meta {

			//
			// Global
			///////////

			static constexpr Name malloc{ "UDRefl::malloc" };
			static constexpr Name free{ "UDRefl::free" };
			static constexpr Name aligned_malloc{ "UDRefl::aligned_malloc" };
			static constexpr Name aligned_free{ "UDRefl::aligned_free" };

			//
			// Member
			///////////

			static constexpr Name ctor{ "UDRefl::ctor" };
			static constexpr Name dtor{ "UDRefl::dtor" };

			static constexpr Name operator_bool{ "UDRefl::operator_bool" };

			static constexpr Name operator_plus{ "UDRefl::operator+" };
			static constexpr Name operator_minus{ "UDRefl::operator-" };

			static constexpr Name operator_add{ "UDRefl::operator+" };
			static constexpr Name operator_sub{ "UDRefl::operator-" };
			static constexpr Name operator_mul{ "UDRefl::operator*" };
			static constexpr Name operator_div{ "UDRefl::operator/" };
			static constexpr Name operator_mod{ "UDRefl::operator%" };

			static constexpr Name operator_bnot{ "UDRefl::operator~" };
			static constexpr Name operator_band{ "UDRefl::operator&" };
			static constexpr Name operator_bor{ "UDRefl::operator|" };
			static constexpr Name operator_bxor{ "UDRefl::operator^" };
			static constexpr Name operator_lshift{ "UDRefl::operator<<" };
			static constexpr Name operator_rshift{ "UDRefl::operator>>" };

			static constexpr Name operator_pre_inc{ "UDRefl::operator++" };
			static constexpr Name operator_pre_dec{ "UDRefl::operator--" };
			static constexpr Name operator_post_inc{ "UDRefl::operator++" };
			static constexpr Name operator_post_dec{ "UDRefl::operator--" };

			static constexpr Name operator_assign{ "UDRefl::operator=" };
			static constexpr Name operator_assign_add{ "UDRefl::operator+=" };
			static constexpr Name operator_assign_sub{ "UDRefl::operator-=" };
			static constexpr Name operator_assign_mul{ "UDRefl::operator*=" };
			static constexpr Name operator_assign_div{ "UDRefl::operator/=" };
			static constexpr Name operator_assign_mod{ "UDRefl::operator%=" };
			static constexpr Name operator_assign_band{ "UDRefl::operator&=" };
			static constexpr Name operator_assign_bor{ "UDRefl::operator|=" };
			static constexpr Name operator_assign_bxor{ "UDRefl::operator^=" };
			static constexpr Name operator_assign_lshift{ "UDRefl::operator<<=" };
			static constexpr Name operator_assign_rshift{ "UDRefl::operator>>=" };

			static constexpr Name operator_eq{ "UDRefl::operator==" };
			static constexpr Name operator_ne{ "UDRefl::operator!=" };
			static constexpr Name operator_lt{ "UDRefl::operator<" };
			static constexpr Name operator_le{ "UDRefl::operator<=" };
			static constexpr Name operator_gt{ "UDRefl::operator>" };
			static constexpr Name operator_ge{ "UDRefl::operator>=" };

			static constexpr Name operator_and{ "UDRefl::operator&&" };
			static constexpr Name operator_or{ "UDRefl::operator||" };
			static constexpr Name operator_not{ "UDRefl::operator!" };

			static constexpr Name operator_subscript{ "UDRefl::operator[]" };
			static constexpr Name operator_deref{ "UDRefl::operator*" };
			static constexpr Name operator_ref{ "UDRefl::operator&" };
			static constexpr Name operator_member{ "UDRefl::operator->" };
			static constexpr Name operator_member_of_pointer{ "UDRefl::operator->*" };

			static constexpr Name operator_call{ "UDRefl::operator()" };
			static constexpr Name operator_comma{ "UDRefl::operator," };

			// tuple

			static constexpr Name tuple_size{ "UDRefl::tuple_size" };
			static constexpr Name tuple_get{ "UDRefl::tuple_get" };

			// container

			static constexpr Name container_assign{ "UDRefl::container_assign" };

			static constexpr Name container_begin{ "UDRefl::container_begin" };
			static constexpr Name container_cbegin{ "UDRefl::container_cbegin" };
			static constexpr Name container_end{ "UDRefl::container_end" };
			static constexpr Name container_cend{ "UDRefl::container_cend" };
			static constexpr Name container_rbegin{ "UDRefl::container_rbegin" };
			static constexpr Name container_crbegin{ "UDRefl::container_crbegin" };
			static constexpr Name container_rend{ "UDRefl::container_rend" };
			static constexpr Name container_crend{ "UDRefl::container_crend" };

			static constexpr Name container_at{ "UDRefl::container_at" };
			static constexpr Name container_data{ "UDRefl::container_data" };
			static constexpr Name container_front{ "UDRefl::container_front" };
			static constexpr Name container_back{ "UDRefl::container_back" };

			static constexpr Name container_empty{ "UDRefl::container_empty" };
			static constexpr Name container_size{ "UDRefl::container_size" };
			static constexpr Name container_max_size{ "UDRefl::container_max_size" };
			static constexpr Name container_resize{ "UDRefl::container_resize" };
			static constexpr Name container_capacity{ "UDRefl::container_capacity" };
			static constexpr Name container_bucket_count{ "UDRefl::container_bucket_count" };
			static constexpr Name container_reserve{ "UDRefl::container_reserve" };
			static constexpr Name container_shrink_to_fit{ "UDRefl::container_shrink_to_fit" };

			static constexpr Name container_clear{ "UDRefl::container_clear" };
			static constexpr Name container_insert{ "UDRefl::container_insert" };
			static constexpr Name container_insert_or_assign{ "UDRefl::container_insert_or_assign" };
			static constexpr Name container_erase{ "UDRefl::container_erase" };
			static constexpr Name container_push_front{ "UDRefl::container_push_front" };
			static constexpr Name container_pop_front{ "UDRefl::container_pop_front" };
			static constexpr Name container_push_back{ "UDRefl::container_push_back" };
			static constexpr Name container_pop_back{ "UDRefl::container_pop_back" };
			static constexpr Name container_swap{ "UDRefl::container_swap" };
			static constexpr Name container_merge{ "UDRefl::container_merge" };
			static constexpr Name container_extract{ "UDRefl::container_extract" };

			static constexpr Name container_count{ "UDRefl::container_count" };
			static constexpr Name container_find{ "UDRefl::container_find" };
			static constexpr Name container_lower_bound{ "UDRefl::container_lower_bound" };
			static constexpr Name container_upper_bound{ "UDRefl::container_upper_bound" };
			static constexpr Name container_equal_range{ "UDRefl::container_equal_range" };

			static constexpr Name container_key_comp{ "UDRefl::container_key_comp" };
			static constexpr Name container_value_comp{ "UDRefl::container_value_comp" };
			static constexpr Name container_hash_function{ "UDRefl::container_hash_function" };
			static constexpr Name container_key_eq{ "UDRefl::container_key_eq" };
			static constexpr Name container_get_allocator{ "UDRefl::container_get_allocator" };
		};

		NameIDRegistry();
	};

	class TypeIDRegistry : public IDRegistry<TypeID> {
	public:
		struct Meta {
			static constexpr Type global{ "UDRefl::global" };
			static constexpr Type t_void{ "void" };
		};

		TypeIDRegistry();

		void   RegisterUnmanaged(TypeID ID, std::string_view name);
		TypeID RegisterUnmanaged(           std::string_view name);
		void   Register         (TypeID ID, std::string_view name);
		TypeID Register         (           std::string_view name);

		// unmanaged
		// U = std::remove_cvref_t<T>
		// register: U, const U, U&, U&&, const U&, const U&&
		template<typename T>
		void Register();

		template<typename T>
		bool IsRegistered() const;

		//
		// Type Computation
		/////////////////////

		Type RegisterAddConst(Type type);
		Type RegisterAddLValueReference(Type type);
		Type RegisterAddLValueReferenceWeak(Type type);
		Type RegisterAddRValueReference(Type type);
		Type RegisterAddConstLValueReference(Type type);
		Type RegisterAddConstRValueReference(Type type);

	private:
		using IDRegistry<TypeID>::Register;
		using IDRegistry<TypeID>::IsRegistered;
	};
}

#include "details/IDRegistry.inl"
