#pragma once

#include "ObjectPtr.h"
#include "Util.h"

#include <variant>

namespace Ubpa::UDRefl {
	class FieldPtr {
	public:
		FieldPtr(TypeID valueID, size_t forward_offset_value, bool isConst) noexcept :
			valueID{ valueID }
		{
			assert(valueID);

			if (isConst)
				offset.emplace<0>(forward_offset_value);
			else
				offset.emplace<1>(forward_offset_value);
		}

		constexpr FieldPtr(TypeID valueID, size_t forward_offset_value, std::true_type is_const) noexcept :
			valueID{ valueID },
			offset{ std::in_place_index_t<1>{}, forward_offset_value }
		{ assert(valueID); }

		constexpr FieldPtr(TypeID valueID, size_t forward_offset_value, std::false_type is_const) noexcept :
			valueID{ valueID },
			offset{ std::in_place_index_t<0>{}, forward_offset_value }
		{ assert(valueID); }

		constexpr FieldPtr(TypeID valueID, size_t forward_offset_value) noexcept
			: FieldPtr{ valueID, forward_offset_value, std::false_type{} } {}

		constexpr FieldPtr(TypeID valueID, void* ptr) noexcept :
			valueID{ valueID },
			offset{ ptr }
		{ assert(valueID&& ptr); }

		constexpr FieldPtr(TypeID valueID, const void* ptr) noexcept :
			valueID{ valueID },
			offset{ ptr }
		{ assert(valueID&& ptr); }

		constexpr FieldPtr(ObjectPtr      static_obj) noexcept : FieldPtr{ static_obj.GetID(), static_obj.GetPtr() } {}
		constexpr FieldPtr(ConstObjectPtr static_obj) noexcept : FieldPtr{ static_obj.GetID(), static_obj.GetPtr() } {}

		FieldPtr(TypeID valueID, Offsetor offsetor, bool isConst) noexcept :
			valueID{ valueID }
		{
			assert(valueID && offsetor);

			if (isConst)
				offset.emplace<5>(std::move(offsetor));
			else
				offset.emplace<4>(std::move(offsetor));
		}

		FieldPtr(TypeID valueID, Offsetor offsetor, std::true_type is_const) noexcept :
			valueID{ valueID },
			offset{ std::in_place_index_t<5>{}, std::move(offsetor) }
		{
			assert(valueID&& std::get<5>(offset));
		}

		FieldPtr(TypeID valueID, Offsetor offsetor, std::false_type is_const) noexcept :
			valueID{ valueID },
			offset{ std::in_place_index_t<4>{}, std::move(offsetor) }
		{
			assert(valueID&& std::get<4>(offset));
		}

		FieldPtr(TypeID valueID, Offsetor offsetor) noexcept : FieldPtr{ valueID, std::move(offsetor), std::false_type{} } {}

		constexpr TypeID GetValueID() const noexcept { return valueID; }

		constexpr bool IsBasic()    const noexcept { return offset.index() == 0 || offset.index() == 1; }
		constexpr bool IsStatic()   const noexcept { return offset.index() == 2 || offset.index() == 3; }
		constexpr bool IsVirtual()  const noexcept { return offset.index() == 4 || offset.index() == 5; }
		constexpr bool IsConst()    const noexcept { return offset.index() & 1; }
		constexpr bool IsVariable() const noexcept { return !IsConst(); }

		constexpr bool IsBasicVaraible()   const noexcept { return offset.index() == 0; }
		constexpr bool IsBasicConst()      const noexcept { return offset.index() == 1; }
		constexpr bool IsStaticVaraible()  const noexcept { return offset.index() == 2; }
		constexpr bool IsStaticConst()     const noexcept { return offset.index() == 3; }
		constexpr bool IsVirtualVaraible() const noexcept { return offset.index() == 4; }
		constexpr bool IsVirtualConst()    const noexcept { return offset.index() == 5; }

		constexpr ObjectPtr Map_BasicVariable(void* obj) const noexcept {
			assert(IsBasicVaraible());
			return { valueID, forward_offset(obj, std::get<0>(offset)) };
		}

		constexpr ConstObjectPtr Map_BasicConst(const void* obj) const noexcept {
			assert(IsBasicConst());
			return { valueID, forward_offset(obj, std::get<1>(offset)) };
		}

		constexpr ObjectPtr Map_StaticVariable() const noexcept {
			assert(IsStaticVaraible());
			return { valueID, std::get<2>(offset) };
		}

		constexpr ConstObjectPtr Map_StaticConst() const noexcept {
			assert(IsStaticConst());
			return { valueID, std::get<3>(offset) };
		}

		constexpr ObjectPtr Map_VirtualVariable(void* obj) const noexcept {
			assert(IsVirtualVaraible());
			return { valueID, const_cast<void*>(std::get<4>(offset)(obj)) };
		}

		constexpr ConstObjectPtr Map_VirtualConst(const void* obj) const noexcept {
			assert(IsVirtualConst());
			return { valueID, std::get<5>(offset)(obj) };
		}

		// static { variable | const }
		ConstObjectPtr Map() const noexcept {
			return std::visit([this](const auto& offset) -> ConstObjectPtr {
				using T = std::decay_t<decltype(offset)>;
				if constexpr (std::is_same_v<T, size_t>) {
					assert(false);
					return nullptr;
				}
				else if constexpr (std::is_same_v<T, void*>) {
					return { valueID, offset };
				}
				else if constexpr (std::is_same_v<T, const void*>) {
					return { valueID, offset };
				}
				else if constexpr (std::is_same_v<T, Offsetor>) {
					assert(false);
					return nullptr;
				}
				else
					static_assert(false);
			}, offset);
		}

		// all
		ConstObjectPtr Map(const void* obj) const noexcept {
			return std::visit([this, obj](const auto& offset) -> ConstObjectPtr {
				using T = std::decay_t<decltype(offset)>;
				if constexpr (std::is_same_v<T, size_t>) {
					assert(obj);
					return { valueID, forward_offset(obj, offset) };
				}
				else if constexpr (std::is_same_v<T, void*>) {
					return { valueID, offset };
				}
				else if constexpr (std::is_same_v<T, const void*>) {
					return { valueID, offset };
				}
				else if constexpr (std::is_same_v<T, Offsetor>) {
					assert(obj);
					return { valueID, offset(obj) };
				}
				else
					static_assert(false);
			}, offset);
		}

		// {normal | static | virutal } variable
		constexpr ObjectPtr Map(void* obj) const noexcept {
			switch (offset.index())
			{
			case 0:
				assert(obj);
				return { valueID, forward_offset(obj, std::get<0>(offset)) };
			case 2:
				return { valueID, std::get<2>(offset) };
			case 4:
				assert(obj);
				return { valueID, const_cast<void*>(std::get<4>(offset)(obj)) };
			default:
				assert("require variable" && false);
				return nullptr;
			}
		}

	private:
		TypeID valueID;

		std::variant<
			size_t,       // forward_offset_value 0 BASIC_VARIABLE
			size_t,       // forward_offset_value 1 BASIC_CONST
			void*,        // static_obj           2 STATIC_VARIABLE
			const void*,  // static_const_obj     3 STATIC_CONST
			Offsetor,     // offsetor             4 VIRTUAL_VARIABLE
			Offsetor      // offsetor             5 VIRTUAL_CONST
		> offset;
	};
}
