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
		using Data = 
			std::variant<
				size_t,             // forward_offset_value 0 BASIC_VARIABLE
				size_t,             // forward_offset_value 1 BASIC_CONST
				Offsetor,           // offsetor             2 VIRTUAL_VARIABLE
				Offsetor,           // offsetor             3 VIRTUAL_CONST
				void*,              // static_obj           4 STATIC_VARIABLE
				const void*,        // static_const_obj     5 STATIC_CONST
				SharedBuffer,       // dynamic_obj          6 DYNAMIC_SHARED_VARIABLE
				SharedConstBuffer,  // dynamic_obj          7 DYNAMIC_SHARED_CONST
				Buffer,             // dynamic_obj          8 DYNAMIC_BUFFER_VARIABLE
				const Buffer        // dynamic_obj          9 DYNAMIC_BUFFER_CONST
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

		FieldPtr(TypeID valueID, size_t forward_offset_value, bool isConst) noexcept;

		constexpr FieldPtr(TypeID valueID, size_t forward_offset_value, std::true_type is_const) noexcept :
			valueID{ valueID },
			data{ std::in_place_index_t<1>{}, forward_offset_value }
		{ assert(valueID); }

		constexpr FieldPtr(TypeID valueID, size_t forward_offset_value, std::false_type is_const) noexcept :
			valueID{ valueID },
			data{ std::in_place_index_t<0>{}, forward_offset_value }
		{ assert(valueID); }

		constexpr FieldPtr(TypeID valueID, size_t forward_offset_value) noexcept
			: FieldPtr{ valueID, forward_offset_value, std::false_type{} } {}

		FieldPtr(TypeID valueID, Offsetor offsetor, bool isConst) noexcept :
			valueID{ valueID }
		{
			assert(valueID && offsetor);

			if (isConst)
				data.emplace<3>(std::move(offsetor));
			else
				data.emplace<2>(std::move(offsetor));
		}

		FieldPtr(TypeID valueID, Offsetor offsetor, std::true_type is_const) noexcept :
			valueID{ valueID },
			data{ std::in_place_index_t<3>{}, std::move(offsetor) }
		{
			assert(valueID && std::get<3>(data));
		}

		FieldPtr(TypeID valueID, Offsetor offsetor, std::false_type is_const) noexcept :
			valueID{ valueID },
			data{ std::in_place_index_t<2>{}, std::move(offsetor) }
		{
			assert(valueID && std::get<2>(data));
		}

		FieldPtr(TypeID valueID, Offsetor offsetor) noexcept : FieldPtr{ valueID, std::move(offsetor), std::false_type{} } {}

		constexpr FieldPtr(TypeID valueID, void* ptr) noexcept :
			valueID{ valueID },
			data{ ptr }
		{ assert(valueID && ptr); }

		constexpr FieldPtr(TypeID valueID, const void* ptr) noexcept :
			valueID{ valueID },
			data{ ptr }
		{ assert(valueID && ptr); }

		explicit constexpr FieldPtr(ObjectPtr      static_obj) noexcept : FieldPtr{ static_obj.GetID(), static_obj.GetPtr() } {}
		explicit constexpr FieldPtr(ConstObjectPtr static_obj) noexcept : FieldPtr{ static_obj.GetID(), static_obj.GetPtr() } {}

		explicit FieldPtr(const SharedObject& obj) noexcept :
			valueID{ obj.GetID() },
			data{ std::in_place_index_t<6>{}, obj.GetBuffer() }
		{ assert(obj.Valid()); }

		explicit FieldPtr(SharedObject&& obj) noexcept :
			valueID{ obj.GetID() },
			data{ std::in_place_index_t<6>{}, std::move(obj).GetBuffer() }
		{ assert(std::get<6>(data)); }

		explicit FieldPtr(const SharedConstObject& obj) noexcept :
			valueID{ obj.GetID() },
			data{ std::in_place_index_t<7>{}, obj.GetBuffer() }
		{ assert(obj.Valid()); }

		explicit FieldPtr(SharedConstObject&& obj) noexcept :
			valueID{ obj.GetID() },
			data{ std::in_place_index_t<7>{}, obj.GetBuffer() }
		{ assert(obj.Valid()); }

		FieldPtr(TypeID valueID, const Buffer& buffer, bool isConst) noexcept;

		FieldPtr(TypeID valueID, const Buffer& buffer, std::true_type isConst) noexcept :
			valueID{ valueID },
			data{ std::in_place_index_t<9>{}, buffer }
		{ assert(valueID); }

		FieldPtr(TypeID valueID, const Buffer& buffer, std::false_type isConst) noexcept :
			valueID{ valueID },
			data{ std::in_place_index_t<8>{}, buffer }
		{ assert(valueID); }

		FieldPtr(TypeID valueID, const Buffer& buffer) noexcept : FieldPtr{ valueID, buffer, std::false_type{} } {}

		constexpr TypeID GetValueID() const noexcept { return valueID; }

		constexpr bool IsBasicVaraible()         const noexcept { return data.index() == 0; }
		constexpr bool IsBasicConst()            const noexcept { return data.index() == 1; }
		constexpr bool IsVirtualVaraible()       const noexcept { return data.index() == 2; }
		constexpr bool IsVirtualConst()          const noexcept { return data.index() == 3; }
		constexpr bool IsStaticVaraible()        const noexcept { return data.index() == 4; }
		constexpr bool IsStaticConst()           const noexcept { return data.index() == 5; }
		constexpr bool IsDynamicSharedVaraible() const noexcept { return data.index() == 6; }
		constexpr bool IsDynamicSharedConst()    const noexcept { return data.index() == 7; }
		constexpr bool IsDynamicBufferVaraible() const noexcept { return data.index() == 8; }
		constexpr bool IsDynamicBufferConst()    const noexcept { return data.index() == 9; }

		constexpr bool IsBasic()          const noexcept { return data.index() == 0 || data.index() == 1; }
		constexpr bool IsVirtual()        const noexcept { return data.index() == 2 || data.index() == 3; }
		constexpr bool IsStatic()         const noexcept { return data.index() == 4 || data.index() == 5; }
		constexpr bool IsDynamicShared()  const noexcept { return data.index() == 6 || data.index() == 7; }
		constexpr bool IsDyanmicBuffer()  const noexcept { return data.index() == 8 || data.index() == 9; }

		constexpr bool IsConst()    const noexcept { return data.index() & 1; }
		constexpr bool IsVariable() const noexcept { return !IsConst(); }

		constexpr bool IsOffsetor() const noexcept { return data.index() < 4; }
		constexpr bool IsObject()   const noexcept { return data.index() >= 4; }

		// { variable | const } object
		ConstObjectPtr RVar() const;

		// variable object
		ObjectPtr RWVar();

		// all
		ConstObjectPtr RVar(const void* obj) const;

		// variable
		ObjectPtr RWVar(void* obj) noexcept;

	private:
		TypeID valueID;
		Data data;
	};
}
