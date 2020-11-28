#pragma once

#include "Object.h"
#include "Util.h"

namespace Ubpa::UDRefl {
	class FieldPtr {
	public:
		enum class Mode : std::uint8_t {
			VARIABLE         = 0b000,
			CONST            = 0b001,
			STATIC_VARIABLE  = 0b010,
			STATIC_CONST     = 0b011,
			VIRTUAL_VARIABLE = 0b100,
			VIRTUAL_CONST    = 0b101,
		};

		constexpr FieldPtr(TypeID valueID, size_t offset, bool isConst = false) noexcept :
			mode{ isConst? Mode::CONST : Mode::VARIABLE },
			valueID{ valueID },
			offset{ offset }
		{
			assert(valueID);
		}

		constexpr FieldPtr(TypeID valueID, void* ptr) noexcept :
			mode{ Mode::STATIC_VARIABLE },
			valueID{ valueID },
			static_obj{ ptr }
		{
			assert(valueID && ptr);
		}

		constexpr FieldPtr(TypeID valueID, const void* ptr) noexcept :
			mode{ Mode::STATIC_CONST },
			valueID{ valueID },
			static_const_obj{ ptr }
		{
			assert(valueID && ptr);
		}

		constexpr FieldPtr(ObjectPtr obj) noexcept : FieldPtr{ obj.GetID(), obj.GetPtr() } {}
		constexpr FieldPtr(ConstObjectPtr obj) noexcept : FieldPtr{ obj.GetID(), obj.GetPtr() } {}

		constexpr FieldPtr(TypeID valueID, OffsetFunction* offset_function, bool isConst = false) noexcept :
			mode{ isConst ? Mode::VIRTUAL_CONST : Mode::VIRTUAL_VARIABLE },
			valueID{ valueID },
			offset_function{ offset_function }
		{
			assert(valueID && offset_function);
		}

		constexpr TypeID GetValueID() const noexcept { return valueID; }
		constexpr Mode GetMode() const noexcept { return mode; }

		constexpr bool IsConst()   const noexcept { return static_cast<std::uint8_t>(mode) & 0b001; }
		constexpr bool IsStatic()  const noexcept { return static_cast<std::uint8_t>(mode) & 0b010; }
		constexpr bool IsVirtual() const noexcept { return static_cast<std::uint8_t>(mode) & 0b100; }

		constexpr ObjectPtr Map_Variable(void* obj) const noexcept {
			assert(mode == Mode::VARIABLE);
			return { valueID, forward_offset(obj, offset) };
		}

		constexpr ConstObjectPtr Map_Const(const void* obj) const noexcept {
			assert(mode == Mode::CONST);
			return { valueID, forward_offset(obj, offset) };
		}

		constexpr ObjectPtr Map_StaticVariable() const noexcept {
			assert(mode == Mode::STATIC_VARIABLE);
			return { valueID, static_obj };
		}

		constexpr ConstObjectPtr Map_StaticConst() const noexcept {
			assert(mode == Mode::STATIC_CONST);
			return { valueID, static_const_obj };
		}

		constexpr ObjectPtr Map_VirtualVariable(void* obj) const noexcept {
			assert(mode == Mode::VIRTUAL_VARIABLE);
			return { valueID, const_cast<void*>(offset_function(obj)) };
		}

		constexpr ConstObjectPtr Map_VirtualConst(const void* obj) const noexcept {
			assert(mode == Mode::VIRTUAL_CONST);
			return { valueID, offset_function(obj) };
		}

		// static { variable | const }
		constexpr ConstObjectPtr Map() const noexcept {
			switch (mode)
			{
			case Mode::STATIC_VARIABLE:
				return { valueID, static_obj };
			case Mode::STATIC_CONST:
				return { valueID, static_const_obj };
			default:
				assert(false);
				return nullptr;
			}
		}

		// all
		constexpr ConstObjectPtr Map(const void* obj) const noexcept {
			switch (mode)
			{
			case Mode::VARIABLE:
			case Mode::CONST:
				assert(obj != nullptr);
				return { valueID, forward_offset(obj, offset) };
			case Mode::STATIC_VARIABLE:
				return { valueID, static_obj };
			case Mode::STATIC_CONST:
				return { valueID, static_const_obj };
			case Mode::VIRTUAL_VARIABLE:
			case Mode::VIRTUAL_CONST:
				assert(obj != nullptr);
				return { valueID, offset_function(obj) };
			default:
				assert(false);
				return nullptr;
			}
		}

		// {normal | static | virutal } variable
		constexpr ObjectPtr Map(void* obj) const noexcept {
			switch (mode)
			{
			case Mode::VARIABLE:
				assert(obj != nullptr);
				return { valueID, forward_offset(obj, offset) };
			case Mode::STATIC_VARIABLE:
				return { valueID, static_obj };
			case Mode::VIRTUAL_VARIABLE:
				assert(obj != nullptr);
				return { valueID, const_cast<void*>(offset_function(obj)) };
			default:
				assert(false);
				return nullptr;
			}
		}

	private:
		Mode mode;
		TypeID valueID;
		union {
			size_t offset;
			void* static_obj;
			const void* static_const_obj;
			OffsetFunction* offset_function;
		};
	};
}

#include "details/FieldPtr.inl"
