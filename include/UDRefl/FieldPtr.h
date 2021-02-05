#pragma once

#include "Object.h"

#include <variant>
#include <array>

namespace Ubpa::UDRefl {
	class FieldPtr {
	public:
		//
		// Buffer
		///////////

		static constexpr size_t BufferSize = std::max(sizeof(Offsetor), sizeof(SharedBuffer)); // maybe 64
		using Buffer = std::aligned_storage_t<BufferSize>;
		static_assert(sizeof(Buffer) == BufferSize);
		// raw offsetor
		using Data = 
			std::variant<
				std::size_t,  // forward_offset_value 0 BASIC
				Offsetor,     // offsetor             1 VIRTUAL
				void*,        // static_obj           2 STATIC
				SharedBuffer, // dynamic_obj          3 DYNAMIC_SHARED
				Buffer        // dynamic_obj          4 DYNAMIC_BUFFER
			>;

		template<typename T>
		static constexpr bool IsBufferable() noexcept {
			return std::is_trivially_copyable_v<T> && sizeof(T) <= BufferSize && alignof(T) <= alignof(Buffer);
		}

		template<typename T>
		static constexpr Buffer ConvertToBuffer(const T& data) noexcept {
			static_assert(IsBufferable<T>());
			Buffer buffer{};
			memcpy(&buffer, &data, sizeof(T));
			return buffer;
		}

		//
		// Constructor
		////////////////

		constexpr FieldPtr() noexcept = default;

		constexpr FieldPtr(TypeID valueID, std::size_t forward_offset_value) noexcept :
			valueID{ valueID },
			data{ forward_offset_value }
		{ assert(valueID); }

		FieldPtr(TypeID valueID, Offsetor offsetor) noexcept :
			valueID{ valueID },
			data{ std::move(offsetor) }
		{
			assert(valueID&& std::get<1>(data));
		}

		constexpr FieldPtr(TypeID valueID, void* ptr) noexcept :
			valueID{ valueID },
			data{ ptr }
		{ assert(valueID && ptr); }

		explicit constexpr FieldPtr(ObjectPtr static_obj) noexcept : FieldPtr{ static_obj.GetTypeID(), static_obj.GetPtr() } {}

		explicit FieldPtr(SharedObject obj) noexcept :
			valueID{ obj.GetTypeID() },
			data{ std::move(obj.GetBuffer()) }
		{ assert(valueID && std::get<3>(data)); }

		FieldPtr(TypeID valueID, const Buffer& buffer) noexcept :
			valueID{ valueID },
			data{ buffer }
		{ assert(valueID); }

		constexpr TypeID GetValueID() const noexcept { return valueID; }

		constexpr bool IsBasic()         const noexcept { return data.index() == 0; }
		constexpr bool IsVirtual()       const noexcept { return data.index() == 1; }
		constexpr bool IsStatic()        const noexcept { return data.index() == 2; }
		constexpr bool IsDynamicShared() const noexcept { return data.index() == 3; }
		constexpr bool IsDyanmicBuffer() const noexcept { return data.index() == 4; }

		constexpr bool IsOwned() const noexcept { return data.index() < 2; }
		constexpr bool IsUnowned() const noexcept { return data.index() >= 2; }

		// object
		ObjectPtr Var() noexcept;

		// variable
		ObjectPtr Var(void* obj);

	private:
		TypeID valueID;
		Data data;
	};
}
