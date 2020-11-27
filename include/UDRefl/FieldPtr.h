#pragma once

#include "Object.h"
#include "Util.h"

namespace Ubpa::UDRefl {
	// non-static
	class FieldPtr {
	public:
		using OffsetFunctor = const void*(const void*);
		enum class Type : std::uint8_t {
			VARIABLE         = 0b000,
			CONST            = 0b001,
			STATIC_VARIABLE  = 0b010,
			STATIC_CONST     = 0b011,
			VIRTUAL_VARIABLE = 0b100,
			VIRTUAL_CONST    = 0b101,
		};

		constexpr FieldPtr(size_t valueID, size_t offset, bool isConst = false) noexcept :
			type{ isConst? Type::CONST : Type::VARIABLE },
			valueID{ valueID },
			offset{ offset }
		{
			assert(valueID != static_cast<size_t>(-1));
		}

		constexpr FieldPtr(size_t valueID, void* ptr) noexcept :
			type{ Type::STATIC_VARIABLE },
			valueID{ valueID },
			static_obj{ ptr }
		{
			assert(valueID != static_cast<size_t>(-1) && ptr != nullptr);
		}

		constexpr FieldPtr(size_t valueID, const void* ptr) noexcept :
			type{ Type::STATIC_CONST },
			valueID{ valueID },
			static_const_obj{ ptr }
		{
			assert(valueID != static_cast<size_t>(-1) && ptr != nullptr);
		}

		constexpr FieldPtr(ObjectPtr obj) noexcept : FieldPtr{ obj.GetID(), obj.GetPtr() } {}
		constexpr FieldPtr(ConstObjectPtr obj) noexcept : FieldPtr{ obj.GetID(), obj.GetPtr() } {}

		constexpr FieldPtr(size_t valueID, OffsetFunctor* offset_functor, bool isConst = false) noexcept :
			type{ isConst ? Type::VIRTUAL_CONST : Type::VIRTUAL_VARIABLE },
			valueID{ valueID },
			offset_functor{ offset_functor }
		{
			assert(valueID != static_cast<size_t>(-1) && offset_functor);
		}

		constexpr size_t GetValueID() const noexcept { return valueID; }
		constexpr Type GetType() const noexcept { return type; }

		constexpr bool IsConst()   const noexcept { return static_cast<std::uint8_t>(type) & 0b001; }
		constexpr bool IsStatic()  const noexcept { return static_cast<std::uint8_t>(type) & 0b010; }
		constexpr bool IsVirtual() const noexcept { return static_cast<std::uint8_t>(type) & 0b100; }

		constexpr ObjectPtr Map_Variable(void* obj) const noexcept {
			assert(type == Type::VARIABLE);
			return { valueID, forward_offset(obj, offset) };
		}

		constexpr ConstObjectPtr Map_Const(const void* obj) const noexcept {
			assert(type == Type::CONST);
			return { valueID, forward_offset(obj, offset) };
		}

		constexpr ObjectPtr Map_StaticVariable() const noexcept {
			assert(type == Type::STATIC_VARIABLE);
			return { valueID, static_obj };
		}

		constexpr ConstObjectPtr Map_StaticConst() const noexcept {
			assert(type == Type::STATIC_CONST);
			return { valueID, static_const_obj };
		}

		constexpr ObjectPtr Map_VirtualVariable(void* obj) const noexcept {
			assert(type == Type::VIRTUAL_VARIABLE);
			return { valueID, const_cast<void*>(offset_functor(obj)) };
		}

		constexpr ConstObjectPtr Map_VirtualConst(const void* obj) const noexcept {
			assert(type == Type::VIRTUAL_CONST);
			return { valueID, offset_functor(obj) };
		}

		// static
		constexpr ConstObjectPtr Map() const noexcept {
			switch (type)
			{
			case Type::STATIC_VARIABLE:
				return { valueID, static_obj };
			case Type::STATIC_CONST:
				return { valueID, static_const_obj };
			default:
				assert(false);
				return nullptr;
			}
		}

		// all
		constexpr ConstObjectPtr Map(const void* obj) const noexcept {
			switch (type)
			{
			case Type::VARIABLE:
			case Type::CONST:
				assert(obj != nullptr);
				return { valueID, forward_offset(obj, offset) };
			case Type::STATIC_VARIABLE:
				return { valueID, static_obj };
			case Type::STATIC_CONST:
				return { valueID, static_const_obj };
			case Type::VIRTUAL_VARIABLE:
			case Type::VIRTUAL_CONST:
				assert(obj != nullptr);
				return { valueID, offset_functor(obj) };
			default:
				assert(false);
				return nullptr;
			}
		}

		// variable
		constexpr ObjectPtr Map(void* obj) const noexcept {
			switch (type)
			{
			case Type::VARIABLE:
				assert(obj != nullptr);
				return { valueID, forward_offset(obj, offset) };
			case Type::STATIC_VARIABLE:
				return { valueID, static_obj };
			case Type::VIRTUAL_VARIABLE:
				assert(obj != nullptr);
				return { valueID, const_cast<void*>(offset_functor(obj)) };
			default:
				assert(false);
				return nullptr;
			}
		}

	private:
		Type type;
		size_t valueID;
		union {
			size_t offset;
			void* static_obj;
			const void* static_const_obj;
			OffsetFunctor* offset_functor;
		};
	};
}

#include "details/FieldPtr.inl"
