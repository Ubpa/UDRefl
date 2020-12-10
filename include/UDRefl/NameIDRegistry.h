#pragma once

#include "NameID.h"

namespace Ubpa::UDRefl {
	class NameIDRegistry {
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
			static constexpr char operator_assign_lshift[] = "UDRefl::operator<<";
			static constexpr char operator_assign_rshift[] = "UDRefl::operator>>";

			static constexpr char operator_new[] = "UDRefl::operator new";
			static constexpr char operator_new_array[] = "UDRefl::operator new[]";
			static constexpr char operator_delete[] = "UDRefl::operator delete";
			static constexpr char operator_delete_array[] = "UDRefl::operator delete array";

			static constexpr char operator_member[] = "UDRefl::operator->";
			static constexpr char operator_call[] = "UDRefl::operator()";
			static constexpr char operator_comma[] = "UDRefl::operator,";
			static constexpr char operator_subscript[] = "UDRefl::operator[]";
		};

		NameIDRegistry();

		//
		// API
		////////

		NameID GetID(std::string_view name) { return NameID{ registry.GetID(name) }; }
		std::string_view Nameof(NameID ID) const noexcept { return registry.Nameof(ID.GetValue()); }

	private:
		IDRegistry registry;
	};
}
