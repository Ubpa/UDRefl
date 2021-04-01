#pragma once

#include "Object.hpp"

#include <variant>

namespace Ubpa::UDRefl {
	class UDRefl_core_API FieldPtr {
	public:
		//
		// Buffer
		///////////

		static constexpr std::size_t BufferSize = std::max(sizeof(Offsetor), sizeof(SharedBuffer)); // maybe 64
		using Buffer = std::aligned_storage_t<BufferSize>;
		static_assert(sizeof(Buffer) == BufferSize);
		// raw offsetor
		using Data =  std::variant<
			std::size_t,  // forward_offset_value 0 BASIC
			Offsetor,     // offsetor             1 VIRTUAL
			void*,        // static_obj           2 STATIC
			SharedBuffer, // dynamic_obj          3 DYNAMIC_SHARED
			Buffer        // dynamic_obj          4 DYNAMIC_BUFFER
		>;

		template<typename T>
		static constexpr bool IsBufferable() noexcept {
			return std::is_trivially_copyable_v<T>
				&& sizeof(T) <= BufferSize
				&& alignof(T) <= alignof(Buffer);
		}

		template<typename T>
		static constexpr Buffer ConvertToBuffer(const T& data) noexcept {
			static_assert(IsBufferable<T>());
			Buffer buffer;
			memcpy(&buffer, &data, sizeof(T));
			return buffer;
		}

		//
		// Constructor
		////////////////

		FieldPtr() noexcept = default;

		FieldPtr(Type type, std::size_t forward_offset_value) noexcept :
			type{ type },
			data{ forward_offset_value }
		{ assert(type); }

		FieldPtr(Type type, Offsetor offsetor) noexcept :
			type{ type },
			data{ std::move(offsetor) }
		{ assert(type && std::get<1>(data)); }

		FieldPtr(Type type, void* ptr) noexcept :
			type{ type },
			data{ ptr }
		{ assert(type && ptr); }

		explicit FieldPtr(ObjectView static_obj) noexcept :
			type{ static_obj.GetType() },
			data{ static_obj.GetPtr() }
		{ assert(type && static_obj.GetPtr()); }

		explicit FieldPtr(SharedObject obj) noexcept :
			type{ obj.GetType() },
			data{ std::move(obj.GetBuffer()) }
		{ assert(type && std::get<3>(data)); }

		FieldPtr(Type type, const Buffer& buffer) noexcept :
			type{ type },
			data{ buffer }
		{ assert(type); }

		Type GetType() const noexcept { return type; }

		FieldFlag GetFieldFlag() const noexcept;

		// unowned
		ObjectView Var();

		ObjectView Var(void* obj);

		// unowned without DYNAMIC_BUFFER
		ObjectView Var() const;

		// without DYNAMIC_BUFFER
		ObjectView Var(void* obj) const;

	private:
		Type type;
		Data data;
	};
}
