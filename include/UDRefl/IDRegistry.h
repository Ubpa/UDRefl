#pragma once

#include "Basic.h"

//#include <string>
//#include <unordered_map>
#include <memory_resource>

#include <shared_mutex>

#ifndef NDEBUG
#include <unordered_set>
#endif // !NDEBUG

namespace Ubpa::UDRefl {
	// name must end with 0
	// thread-safe
	template<typename T, typename U>
	class IDRegistry {
	public:
		IDRegistry();

		void RegisterUnmanaged(T ID, std::string_view name);
		T RegisterUnmanaged(std::string_view name);
		std::string_view Register(T ID, std::string_view name);
		U Register(std::string_view name);

		bool IsRegistered(T ID) const;
		std::string_view Viewof(T ID) const;

		void UnregisterUnmanaged(T ID);
		void Clear() noexcept;

	protected:
		std::pmr::polymorphic_allocator<char> get_allocator() { return &resource; }
		mutable std::shared_mutex smutex;
		
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

	class NameIDRegistry : public IDRegistry<NameID, Name> {
	public:
		struct Meta {
			static constexpr Name ctor{ ".ctor" };
			static constexpr Name dtor{ ".dtor" };

			static constexpr Name operator_bool{ ".operator_bool" };

			static constexpr Name operator_plus{ ".operator+" };
			static constexpr Name operator_minus{ ".operator-" };

			static constexpr Name operator_add{ ".operator+" };
			static constexpr Name operator_sub{ ".operator-" };
			static constexpr Name operator_mul{ ".operator*" };
			static constexpr Name operator_div{ ".operator/" };
			static constexpr Name operator_mod{ ".operator%" };

			static constexpr Name operator_bnot{ ".operator~" };
			static constexpr Name operator_band{ ".operator&" };
			static constexpr Name operator_bor{ ".operator|" };
			static constexpr Name operator_bxor{ ".operator^" };
			static constexpr Name operator_lshift{ ".operator<<" };
			static constexpr Name operator_rshift{ ".operator>>" };

			static constexpr Name operator_pre_inc{ ".operator++" };
			static constexpr Name operator_pre_dec{ ".operator--" };
			static constexpr Name operator_post_inc{ ".operator++" };
			static constexpr Name operator_post_dec{ ".operator--" };

			static constexpr Name operator_assign{ ".operator=" };
			static constexpr Name operator_assign_add{ ".operator+=" };
			static constexpr Name operator_assign_sub{ ".operator-=" };
			static constexpr Name operator_assign_mul{ ".operator*=" };
			static constexpr Name operator_assign_div{ ".operator/=" };
			static constexpr Name operator_assign_mod{ ".operator%=" };
			static constexpr Name operator_assign_band{ ".operator&=" };
			static constexpr Name operator_assign_bor{ ".operator|=" };
			static constexpr Name operator_assign_bxor{ ".operator^=" };
			static constexpr Name operator_assign_lshift{ ".operator<<=" };
			static constexpr Name operator_assign_rshift{ ".operator>>=" };

			static constexpr Name operator_eq{ ".operator==" };
			static constexpr Name operator_ne{ ".operator!=" };
			static constexpr Name operator_lt{ ".operator<" };
			static constexpr Name operator_le{ ".operator<=" };
			static constexpr Name operator_gt{ ".operator>" };
			static constexpr Name operator_ge{ ".operator>=" };

			static constexpr Name operator_and{ ".operator&&" };
			static constexpr Name operator_or{ ".operator||" };
			static constexpr Name operator_not{ ".operator!" };

			static constexpr Name operator_subscript{ ".operator[]" };
			static constexpr Name operator_deref{ ".operator*" };
			static constexpr Name operator_ref{ ".operator&" };
			static constexpr Name operator_member{ ".operator->" };
			static constexpr Name operator_member_of_pointer{ ".operator->*" };

			static constexpr Name operator_call{ ".operator()" };
			static constexpr Name operator_comma{ ".operator," };

			// tuple

			static constexpr Name tuple_size{ ".tuple_size" };
			static constexpr Name tuple_get{ ".tuple_get" };
			static constexpr Name tuple_element{ ".tuple_element" };

			// variant

			static constexpr Name variant_index{ ".variant_index" };
			static constexpr Name variant_valueless_by_exception{ ".variant_valueless_by_exception" };
			static constexpr Name variant_holds_alternative{ ".variant_holds_alternative" };
			static constexpr Name variant_get{ ".variant_get" };
			static constexpr Name variant_get_if{ ".variant_get_if" };
			static constexpr Name variant_size{ ".variant_size" };
			static constexpr Name variant_alternative{ ".variant_alternative" };
			static constexpr Name variant_visit_get{ ".variant_visit_get" };

			// optional

			static constexpr Name optional_has_value{ ".optional_has_value" };
			static constexpr Name optional_value{ ".optional_value" };
			static constexpr Name optional_reset{ ".optional_reset" };

			// iterator

			static constexpr Name iterator_advance{ ".iterator_advance" };
			static constexpr Name iterator_distance{ ".iterator_distance" };
			static constexpr Name iterator_next{ ".iterator_next" };
			static constexpr Name iterator_prev{ ".iterator_prev" };

			// container

			static constexpr Name container_assign{ ".container_assign" };

			static constexpr Name container_begin{ ".container_begin" };
			static constexpr Name container_cbegin{ ".container_cbegin" };
			static constexpr Name container_end{ ".container_end" };
			static constexpr Name container_cend{ ".container_cend" };
			static constexpr Name container_rbegin{ ".container_rbegin" };
			static constexpr Name container_crbegin{ ".container_crbegin" };
			static constexpr Name container_rend{ ".container_rend" };
			static constexpr Name container_crend{ ".container_crend" };

			static constexpr Name container_at{ ".container_at" };
			static constexpr Name container_data{ ".container_data" };
			static constexpr Name container_front{ ".container_front" };
			static constexpr Name container_back{ ".container_back" };
			static constexpr Name container_top{ ".container_top" };

			static constexpr Name container_empty{ ".container_empty" };
			static constexpr Name container_size{ ".container_size" };
			static constexpr Name container_size_bytes{ ".container_size_bytes" };
			static constexpr Name container_max_size{ ".container_max_size" };
			static constexpr Name container_resize{ ".container_resize" };
			static constexpr Name container_capacity{ ".container_capacity" };
			static constexpr Name container_bucket_count{ ".container_bucket_count" };
			static constexpr Name container_reserve{ ".container_reserve" };
			static constexpr Name container_shrink_to_fit{ ".container_shrink_to_fit" };

			static constexpr Name container_clear{ ".container_clear" };
			static constexpr Name container_insert{ ".container_insert" };
			static constexpr Name container_insert_after{ ".container_insert_after" };
			static constexpr Name container_insert_or_assign{ ".container_insert_or_assign" };
			static constexpr Name container_erase{ ".container_erase" };
			static constexpr Name container_erase_after{ ".container_erase_after" };
			static constexpr Name container_push_front{ ".container_push_front" };
			static constexpr Name container_pop_front{ ".container_pop_front" };
			static constexpr Name container_push_back{ ".container_push_back" };
			static constexpr Name container_pop_back{ ".container_pop_back" };
			static constexpr Name container_push{ ".container_push" };
			static constexpr Name container_pop{ ".container_pop" };
			static constexpr Name container_swap{ ".container_swap" };
			static constexpr Name container_merge{ ".container_merge" };
			static constexpr Name container_extract{ ".container_extract" };

			static constexpr Name container_splice_after{ ".container_splice_after" };
			static constexpr Name container_splice{ ".container_splice" };
			static constexpr Name container_remove{ ".container_remove" };
			static constexpr Name container_reverse{ ".container_reverse" };
			static constexpr Name container_unique{ ".container_unique" };
			static constexpr Name container_sort{ ".container_sort" };

			static constexpr Name container_count{ ".container_count" };
			static constexpr Name container_find{ ".container_find" };
			static constexpr Name container_lower_bound{ ".container_lower_bound" };
			static constexpr Name container_upper_bound{ ".container_upper_bound" };
			static constexpr Name container_equal_range{ ".container_equal_range" };
		};

		NameIDRegistry();

		using IDRegistry<NameID, Name>::Register;

		Name Register(Name n) { return Register(n.GetID(), n.GetView()); }
		Name Nameof(NameID ID) const;
	};

	class TypeIDRegistry : public IDRegistry<TypeID, Type> {
	public:
		struct Meta {
			static constexpr Type global{ ".global" };
		};

		using IDRegistry<TypeID, Type>::Register;
		using IDRegistry<TypeID, Type>::IsRegistered;

		TypeIDRegistry();

		// unmanaged
		template<typename T>
		void Register();

		Type Register(Type n) { return Register(n.GetID(), n.GetName()); }

		template<typename T>
		bool IsRegistered() const;

		Type Typeof(TypeID ID) const;

		//
		// Type Computation
		/////////////////////

		Type RegisterAddConst(Type type);
		Type RegisterAddLValueReference(Type type);
		Type RegisterAddLValueReferenceWeak(Type type);
		Type RegisterAddRValueReference(Type type);
		Type RegisterAddConstLValueReference(Type type);
		Type RegisterAddConstRValueReference(Type type);
	};
}

#include "details/IDRegistry.inl"
