#pragma once

#include <UTemplate/TypeID.h>

#include <string>
#include <unordered_map>
#include <memory_resource>
#include <deque>

#ifndef NDEBUG
#include <unordered_set>
#endif // !NDEBUG

namespace Ubpa::UDRefl {
	template<typename T>
	class IDRegistry {
	protected:
		void RegisterUnmanaged(T ID, std::string_view name);
		void Register(T ID, std::string_view name);

	public:
		IDRegistry();

		bool IsRegistered(T ID) const noexcept;
		std::string_view Nameof(T ID) const noexcept;

		void UnregisterUnmanaged(T ID) noexcept;
		void Clear() noexcept;

	private:
		std::pmr::monotonic_buffer_resource resource;
		std::pmr::unordered_map<T, std::string_view> id2name;

#ifndef NDEBUG
	public:
		bool IsUnmanaged(T ID) const noexcept;
		void ClearUnmanaged() noexcept;
	private:
		std::pmr::unordered_set<T> unmanagedIDs;
#endif // NDEBUG
	};

	class StrIDRegistry : public IDRegistry<StrID> {
	public:
		struct Meta {
			//
			// Global
			///////////

			static constexpr char malloc[] = "UDRefl::malloc";
			static constexpr char free[] = "UDRefl::free";
			static constexpr char aligned_malloc[] = "UDRefl::aligned_malloc";
			static constexpr char aligned_free[] = "UDRefl::aligned_free";

			//
			// Member
			///////////

			static constexpr char ctor[] = "UDRefl::ctor";
			static constexpr char dtor[] = "UDRefl::dtor";

			static constexpr char operator_add[] = "UDRefl::operator+";
			static constexpr char operator_minus[] = "UDRefl::operator-";
			static constexpr char operator_mul[] = "UDRefl::operator*";
			static constexpr char operator_div[] = "UDRefl::operator/";
			static constexpr char operator_mod[] = "UDRefl::operator%";

			static constexpr char operator_eq[] = "UDRefl::operator=";
			static constexpr char operator_ne[] = "UDRefl::operator!=";
			static constexpr char operator_lt[] = "UDRefl::operator<";
			static constexpr char operator_le[] = "UDRefl::operator<=";
			static constexpr char operator_gt[] = "UDRefl::operator>";
			static constexpr char operator_ge[] = "UDRefl::operator>=";

			static constexpr char operator_and[] = "UDRefl::operator&&";
			static constexpr char operator_or[] = "UDRefl::operator||";
			static constexpr char operator_not[] = "UDRefl::operator!";

			static constexpr char operator_pos[] = "UDRefl::operator+";
			static constexpr char operator_neg[] = "UDRefl::operator-";
			static constexpr char operator_ref[] = "UDRefl::operator&";
			static constexpr char operator_deref[] = "UDRefl::operator*";

			static constexpr char operator_inc[] = "UDRefl::operator++";
			static constexpr char operator_dec[] = "UDRefl::operator--";

			static constexpr char operator_band[] = "UDRefl::operator&";
			static constexpr char operator_bor[] = "UDRefl::operator|";
			static constexpr char operator_bnot[] = "UDRefl::operator~";
			static constexpr char operator_bxor[] = "UDRefl::operator^";
			static constexpr char operator_lshift[] = "UDRefl::operator<<";
			static constexpr char operator_rshift[] = "UDRefl::operator>>";

			static constexpr char operator_assign[] = "UDRefl::operator=";
			static constexpr char operator_assign_add[] = "UDRefl::operator+=";
			static constexpr char operator_assign_minus[] = "UDRefl::operator-=";
			static constexpr char operator_assign_mul[] = "UDRefl::operator*=";
			static constexpr char operator_assign_div[] = "UDRefl::operator/=";
			static constexpr char operator_assign_mod[] = "UDRefl::operator%=";
			static constexpr char operator_assign_band[] = "UDRefl::operator&=";
			static constexpr char operator_assign_bor[] = "UDRefl::operator|=";
			static constexpr char operator_assign_bxor[] = "UDRefl::operator^=";
			static constexpr char operator_assign_lshift[] = "UDRefl::operator<<=";
			static constexpr char operator_assign_rshift[] = "UDRefl::operator>>=";

			static constexpr char operator_new[] = "UDRefl::operator new";
			static constexpr char operator_new_array[] = "UDRefl::operator new[]";
			static constexpr char operator_delete[] = "UDRefl::operator delete";
			static constexpr char operator_delete_array[] = "UDRefl::operator delete array";

			static constexpr char operator_member[] = "UDRefl::operator->";
			static constexpr char operator_call[] = "UDRefl::operator()";
			static constexpr char operator_comma[] = "UDRefl::operator,";
			static constexpr char operator_subscript[] = "UDRefl::operator[]";
		};
		struct MetaID {
			//
			// Global
			///////////

			static constexpr StrID malloc = Meta::malloc;
			static constexpr StrID free = Meta::free;
			static constexpr StrID aligned_malloc = Meta::aligned_malloc;
			static constexpr StrID aligned_free = Meta::aligned_free;

			//
			// Member
			///////////

			static constexpr StrID ctor = Meta::ctor;
			static constexpr StrID dtor = Meta::dtor;

			static constexpr StrID operator_add = Meta::operator_add;
			static constexpr StrID operator_minus = Meta::operator_minus;
			static constexpr StrID operator_mul = Meta::operator_mul;
			static constexpr StrID operator_div = Meta::operator_div;
			static constexpr StrID operator_mod = Meta::operator_mod;

			static constexpr StrID operator_eq = Meta::operator_eq;
			static constexpr StrID operator_ne = Meta::operator_ne;
			static constexpr StrID operator_lt = Meta::operator_lt;
			static constexpr StrID operator_le = Meta::operator_le;
			static constexpr StrID operator_gt = Meta::operator_gt;
			static constexpr StrID operator_ge = Meta::operator_ge;

			static constexpr StrID operator_and = Meta::operator_and;
			static constexpr StrID operator_or = Meta::operator_or;
			static constexpr StrID operator_not = Meta::operator_not;

			static constexpr StrID operator_pos = Meta::operator_pos;
			static constexpr StrID operator_neg = Meta::operator_neg;
			static constexpr StrID operator_ref = Meta::operator_ref;
			static constexpr StrID operator_deref = Meta::operator_deref;

			static constexpr StrID operator_inc = Meta::operator_inc;
			static constexpr StrID operator_dec = Meta::operator_dec;

			static constexpr StrID operator_band = Meta::operator_band;
			static constexpr StrID operator_bor = Meta::operator_bor;
			static constexpr StrID operator_bnot = Meta::operator_bnot;
			static constexpr StrID operator_bxor = Meta::operator_bxor;
			static constexpr StrID operator_lshift = Meta::operator_lshift;
			static constexpr StrID operator_rshift = Meta::operator_rshift;

			static constexpr StrID operator_assign = Meta::operator_assign;
			static constexpr StrID operator_assign_add = Meta::operator_assign_add;
			static constexpr StrID operator_assign_minus = Meta::operator_assign_minus;
			static constexpr StrID operator_assign_mul = Meta::operator_assign_mul;
			static constexpr StrID operator_assign_div = Meta::operator_assign_div;
			static constexpr StrID operator_assign_mod = Meta::operator_assign_mod;
			static constexpr StrID operator_assign_band = Meta::operator_assign_band;
			static constexpr StrID operator_assign_bor = Meta::operator_assign_bor;
			static constexpr StrID operator_assign_bxor = Meta::operator_assign_bxor;
			static constexpr StrID operator_assign_lshift = Meta::operator_assign_lshift;
			static constexpr StrID operator_assign_rshift = Meta::operator_assign_rshift;

			static constexpr StrID operator_new = Meta::operator_new;
			static constexpr StrID operator_new_array = Meta::operator_new_array;
			static constexpr StrID operator_delete = Meta::operator_delete;
			static constexpr StrID operator_delete_array = Meta::operator_delete_array;

			static constexpr StrID operator_member = Meta::operator_member;
			static constexpr StrID operator_call = Meta::operator_call;
			static constexpr StrID operator_comma = Meta::operator_comma;
			static constexpr StrID operator_subscript = Meta::operator_subscript;
		};

		StrIDRegistry();

		StrID RegisterUnmanaged(std::string_view name) { StrID ID{ name }; IDRegistry<StrID>::RegisterUnmanaged(ID, name); return ID; }
		StrID Register(std::string_view name) { StrID ID{ name }; IDRegistry<StrID>::Register(ID, name); return ID; }
	};

	class TypeIDRegistry : public IDRegistry<TypeID> {
	public:
		struct Meta {
			static constexpr char global[] = "UDRefl::global";
			static constexpr char t_void[] = "void";
		};
		struct MetaID {
			static constexpr TypeID global = Meta::global;
			static constexpr TypeID t_void = Meta::t_void;
		};

		TypeIDRegistry() { RegisterUnmanaged(Meta::global); }

		TypeID RegisterUnmanaged(std::string_view name) { TypeID ID{ name }; IDRegistry<TypeID>::RegisterUnmanaged(ID, name); return ID; }
		TypeID Register(std::string_view name) { TypeID ID{ name }; IDRegistry<TypeID>::Register(ID, name); return ID; }

		// unmanaged
		template<typename T>
		void Register() { IDRegistry<TypeID>::RegisterUnmanaged(TypeID::of<T>, type_name<T>()); }

		template<typename T>
		bool IsRegistered() const noexcept { return IDRegistry<TypeID>::IsRegistered(TypeID::of<T>); }
	};
}

#include "details/IDRegistry.inl"
