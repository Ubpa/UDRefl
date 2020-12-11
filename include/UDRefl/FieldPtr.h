#pragma once

#include "SharedObject.h"
#include "Util.h"

#include <variant>
#include <array>

namespace Ubpa::UDRefl {
	class FieldPtr {
	public:
		//
		// Buffer
		///////////

		static constexpr size_t BufferSize = std::max(sizeof(Offsetor), sizeof(SharedBuffer)); // maybe 64
		using Buffer = std::array<std::uint8_t, sizeof(BufferSize)>;
		using Data = 
			std::variant<
				size_t,             // forward_offset_value 0 BASIC_VARIABLE
				size_t,             // forward_offset_value 1 BASIC_CONST
				Offsetor,           // offsetor             2 VIRTUAL_VARIABLE
				Offsetor,           // offsetor             3 VIRTUAL_CONST
				void*,              // static_obj           4 STATIC_VARIABLE
				const void*,        // static_const_obj     5 STATIC_CONST
				SharedBuffer,       // dynamic_obj          6 DYNAMIC_SHARED_VARIABLE
				const SharedBuffer, // dynamic_obj          7 DYNAMIC_SHARED_CONST
				Buffer,             // dynamic_obj          8 DYNAMIC_BUFFER_VARIABLE
				const Buffer        // dynamic_obj          9 DYNAMIC_BUFFER_CONST
			>;

		template<typename T>
		static constexpr bool IsBufferable() noexcept {
			return std::is_trivial_v<T> && sizeof(T) <= BufferSize;
		}

		template<typename T>
		static constexpr Buffer ConvertToBuffer(const T& data) noexcept {
			static_assert(IsBufferable<T>());
			Buffer buffer{};
			memcpy(buffer.data(), &data, sizeof(T));
			return buffer;
		}

		//
		// Constructor
		////////////////

		constexpr FieldPtr() noexcept = default;

		FieldPtr(TypeID valueID, size_t forward_offset_value, bool isConst) noexcept :
			valueID{ valueID }
		{
			assert(valueID);

			if (isConst)
				data.emplace<0>(forward_offset_value);
			else
				data.emplace<1>(forward_offset_value);
		}

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

		explicit FieldPtr(SharedObject& obj) noexcept :
			valueID{ obj.GetID() },
			data{ std::in_place_index_t<6>{}, obj.GetBuffer() }
		{ assert(obj); }

		explicit FieldPtr(SharedObject&& obj) noexcept :
			valueID{ obj.GetID() },
			data{ std::in_place_index_t<6>{}, std::move(obj).GetBuffer() }
		{ assert(std::get<6>(data)); }

		explicit FieldPtr(const SharedObject& obj) noexcept :
			valueID{ obj.GetID() },
			data{ std::in_place_index_t<7>{}, obj.GetBuffer() }
		{ assert(obj); }

		FieldPtr(TypeID valueID, Buffer& buffer) noexcept :
			valueID{ valueID },
			data{ std::in_place_index_t<8>{}, buffer }
		{ assert(valueID); }

		FieldPtr(TypeID valueID, Buffer&& buffer) noexcept :
			valueID{ valueID },
			data{ std::in_place_index_t<8>{}, buffer }
		{ assert(valueID); }

		FieldPtr(TypeID valueID, const Buffer& buffer) noexcept :
			valueID{ valueID },
			data{ std::in_place_index_t<9>{}, buffer }
		{ assert(valueID); }

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
		constexpr bool IsDyanmicShared()  const noexcept { return data.index() == 6 || data.index() == 7; }
		constexpr bool IsDyanmicBuffer()  const noexcept { return data.index() == 8 || data.index() == 9; }

		constexpr bool IsConst()    const noexcept { return data.index() & 1; }
		constexpr bool IsVariable() const noexcept { return !IsConst(); }

		constexpr bool IsOffsetor() const noexcept { return data.index() < 4; }
		constexpr bool IsObject()   const noexcept { return data.index() >= 4; }

		constexpr ObjectPtr Map_BasicVariable(void* obj) const noexcept {
			assert(IsBasicVaraible());
			return { valueID, forward_offset(obj, std::get<0>(data)) };
		}

		constexpr ConstObjectPtr Map_BasicConst(const void* obj) const noexcept {
			assert(IsBasicConst());
			return { valueID, forward_offset(obj, std::get<1>(data)) };
		}

		constexpr ObjectPtr Map_VirtualVariable(void* obj) const noexcept {
			assert(IsVirtualVaraible());
			return { valueID, const_cast<void*>(std::get<2>(data)(obj)) };
		}

		constexpr ConstObjectPtr Map_VirtualConst(const void* obj) const noexcept {
			assert(IsVirtualConst());
			return { valueID, std::get<3>(data)(obj) };
		}

		constexpr ObjectPtr Map_StaticVariable() const noexcept {
			assert(IsStaticVaraible());
			return { valueID, std::get<4>(data) };
		}

		constexpr ConstObjectPtr Map_StaticConst() const noexcept {
			assert(IsStaticConst());
			return { valueID, std::get<5>(data) };
		}

		constexpr ObjectPtr Map_DynamicSharedVariable() noexcept {
			assert(IsDynamicSharedVaraible());
			return { valueID, std::get<6>(data).Get() };
		}

		constexpr ConstObjectPtr Map_DynamicSharedConst() const noexcept {
			assert(IsDynamicSharedConst());
			return { valueID, std::get<7>(data).Get() };
		}

		constexpr ObjectPtr Map_DynamicBufferVariable() noexcept {
			assert(IsDynamicBufferVaraible());
			return { valueID, std::get<8>(data).data() };
		}

		constexpr ConstObjectPtr Map_DynamicBufferConst() const noexcept {
			assert(IsDynamicBufferConst());
			return { valueID, std::get<9>(data).data() };
		}

		// variable object
		ObjectPtr Map() noexcept {
			return std::visit([this](auto& value) -> ObjectPtr {
				using T = std::remove_reference_t<decltype(value)>;
				if constexpr (std::is_same_v<T, size_t>) {
					assert(false);
					return nullptr;
				}
				else if constexpr (std::is_same_v<T, Offsetor>) {
					assert(false);
					return nullptr;
				}
				else if constexpr (std::is_same_v<T, void*>) {
					return { valueID, value };
				}
				else if constexpr (std::is_same_v<T, const void*>) {
					assert(false);
					return nullptr;
				}
				else if constexpr (std::is_same_v<T, SharedBuffer>) {
					return { valueID, value.Get() };
				}
				else if constexpr (std::is_same_v<T, const SharedBuffer>) {
					assert(false);
					return nullptr;
				}
				else if constexpr (std::is_same_v<T, Buffer>) {
					return { valueID, value.data() };
				}
				else if constexpr (std::is_same_v<T, const Buffer>) {
					assert(false);
					return nullptr;
				}
				else
					static_assert(false);
			}, data);
		}

		// { variable | const } object
		ConstObjectPtr Map() const noexcept {
			return std::visit([this](const auto& value) -> ConstObjectPtr {
				using T = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<T, size_t>) {
					assert(false);
					return nullptr;
				}
				else if constexpr (std::is_same_v<T, Offsetor>) {
					assert(false);
					return nullptr;
				}
				else if constexpr (std::is_same_v<T, void*>) {
					return { valueID, value };
				}
				else if constexpr (std::is_same_v<T, const void*>) {
					return { valueID, value };
				}
				else if constexpr (std::is_same_v<T, SharedBuffer>) {
					return { valueID, value.Get() };
				}
				else if constexpr (std::is_same_v<T, const SharedBuffer>) {
					return { valueID, value.Get() };
				}
				else if constexpr (std::is_same_v<T, Buffer>) {
					return { valueID, value.data() };
				}
				else if constexpr (std::is_same_v<T, const Buffer>) {
					return { valueID, value.data() };
				}
				else
					static_assert(false);
			}, data);
		}

		// all
		ConstObjectPtr Map(const void* obj) const noexcept {
			return std::visit([this, obj](const auto& value) -> ConstObjectPtr {
				using T = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<T, size_t>) {
					assert(obj);
					return { valueID, forward_offset(obj, value) };
				}
				else if constexpr (std::is_same_v<T, Offsetor>) {
					assert(obj);
					return { valueID, value(obj) };
				}
				else if constexpr (std::is_same_v<T, void*>) {
					return { valueID, value };
				}
				else if constexpr (std::is_same_v<T, const void*>) {
					return { valueID, value };
				}
				else if constexpr (std::is_same_v<T, SharedBuffer>) {
					return { valueID, value.Get() };
				}
				else if constexpr (std::is_same_v<T, const SharedBuffer>) {
					return { valueID, value.Get() };
				}
				else if constexpr (std::is_same_v<T, Buffer>) {
					return { valueID, value.data() };
				}
				else if constexpr (std::is_same_v<T, const Buffer>) {
					return { valueID, value.data() };
				}
				else
					static_assert(false);
			}, data);
		}

		// variable
		constexpr ObjectPtr Map(void* obj) noexcept {
			switch (data.index())
			{
			case 0:
				assert(obj);
				return { valueID, forward_offset(obj, std::get<0>(data)) };
			case 2:
				assert(obj);
				return { valueID, const_cast<void*>(std::get<2>(data)(obj)) };
			case 4:
				return { valueID, std::get<4>(data) };
			case 6:
				return { valueID, std::get<6>(data).Get() };
			case 8:
				return { valueID, std::get<8>(data).data() };
			default:
				assert("require variable" && false);
				return nullptr;
			}
		}

	private:
		TypeID valueID;
		Data data;
	};
}
