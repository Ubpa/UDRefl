#pragma once

#include "ID.h"

#include <string>
#include <deque>
#include <unordered_map>

namespace Ubpa::UDRefl {
	class Registry {
	public:
		size_t Register(std::string_view name);
		size_t GetID(std::string_view name) const noexcept;
		bool IsRegistered(size_t ID) const noexcept { return ID < names.size(); }
		std::string_view Nameof(size_t ID) const noexcept;

	private:
		std::deque<std::string> names;
		std::unordered_map<std::string_view, size_t> name2id;
	};

	class NameRegistry {
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
			// Common
			///////////

			static constexpr char size[] = "size";
			static constexpr char alignment[] = "alignment";
			static constexpr char ptr[] = "ptr";

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

		NameRegistry();

		//
		// API
		////////

		NameID Register(std::string_view name) { return NameID{ registry.Register(name) }; }
		NameID GetID(std::string_view name) const noexcept { return NameID{ registry.GetID(name) }; }
		bool IsRegistered(NameID ID) const noexcept { return registry.IsRegistered(ID.GetValue()); }
		std::string_view Nameof(NameID ID) const noexcept { return registry.Nameof(ID.GetValue()); }

	private:
		Registry registry;
	};

	class TypeRegistry {
	public:
		struct Meta {
			static constexpr char global[] = "UDRefl::global";

			//
			// Float
			//////////

			static constexpr char t_float[] = "float";
			static constexpr char t_double[] = "double";

			//
			// Fix Size
			/////////////

			static constexpr char t_int8_t[] = "int8_t";
			static constexpr char t_int16_t[] = "int16_t";
			static constexpr char t_int32_t[] = "int32_t";
			static constexpr char t_int64_t[] = "int64_t";

			static constexpr char t_uint8_t[] = "uint8_t";
			static constexpr char t_uint16_t[] = "uint16_t";
			static constexpr char t_uint32_t[] = "uint32_t";
			static constexpr char t_uint64_t[] = "uint64_t";

			//
			// Common
			///////////

			static constexpr char t_void_ptr[] = "void*";
			static constexpr char t_const_void_ptr[] = "const void*";
		};

		TypeRegistry();

		TypeID Register(std::string_view name) { return TypeID{ registry.Register(name) }; }
		TypeID GetID(std::string_view name) const noexcept { return TypeID{ registry.GetID(name) }; }
		bool IsRegistered(TypeID ID) const noexcept { return registry.IsRegistered(ID.GetValue()); }
		std::string_view Nameof(TypeID ID) const noexcept { return registry.Nameof(ID.GetValue()); }

	private:
		Registry registry;
	};
}
