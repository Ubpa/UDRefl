#pragma once

#include <cstdint>
#include <any>
#include <unordered_map>

namespace Ubpa::UDRefl {
	struct Enumerator {
		enum class UnderlyingType {
			INT8,
			INT16,
			INT32,
			INT64,
			UINT8,
			UINT16,
			UINT32,
			UINT64,
		};

		struct Value {
			union {
				std::int8_t   data_int8;
				std::int16_t  data_int16;
				std::int32_t  data_int32;
				std::int64_t  data_int64;
				std::uint8_t  data_uint8;
				std::uint16_t data_uint16;
				std::uint32_t data_uint32;
				std::uint64_t data_uint64;
			};
		};

		template<typename E>
		static constexpr UnderlyingType UnderlyingTypeOf() noexcept {
			static_assert(std::is_enum_v<E>);
			using T = std::underlying_type_t<E>;
			if constexpr (std::is_signed_v<T>) {
				if constexpr (sizeof(T) == 1)
					return UnderlyingType::INT8;
				else if constexpr (sizeof(T) == 2)
					return UnderlyingType::INT16;
				else if constexpr (sizeof(T) == 4)
					return UnderlyingType::INT32;
				else if constexpr (sizeof(T) == 8)
					return UnderlyingType::INT64;
				else
					static_assert(true);
			}
			else {
				if constexpr (sizeof(T) == 1)
					return UnderlyingType::UINT8;
				else if constexpr (sizeof(T) == 2)
					return UnderlyingType::UINT16;
				else if constexpr (sizeof(T) == 4)
					return UnderlyingType::UINT32;
				else if constexpr (sizeof(T) == 8)
					return UnderlyingType::UINT64;
				else
					static_assert(true);
			}
		}

		template<typename E>
		static constexpr Value ValueOf(E e) noexcept {
			constexpr auto type = UnderlyingTypeOf<E>();
			Value rst;

			if constexpr (type == UnderlyingType::INT8)
				rst.data_int8 = static_cast<std::int8_t>(e);
			else if constexpr (type == UnderlyingType::INT16)
				rst.data_int16 = static_cast<std::int16_t>(e);
			else if constexpr (type == UnderlyingType::INT32)
				rst.data_int32 = static_cast<std::int32_t>(e);
			else if constexpr (type == UnderlyingType::INT64)
				rst.data_int64 = static_cast<std::int64_t>(e);
			else if constexpr (type == UnderlyingType::INT8)
				rst.data_uint8 = static_cast<std::uint8_t>(e);
			else if constexpr (type == UnderlyingType::INT16)
				rst.data_uint16 = static_cast<std::uint16_t>(e);
			else if constexpr (type == UnderlyingType::INT32)
				rst.data_uint32 = static_cast<std::uint32_t>(e);
			else if constexpr (type == UnderlyingType::INT64)
				rst.data_uint64 = static_cast<std::uint64_t>(e);
			else
				static_assert(true);

			return rst;
		}

		UnderlyingType underlyingType;
		Value value;
	};

	struct EnumeratorInfo {
		template<typename E>
		EnumeratorInfo(E e, std::unordered_map<size_t, std::any> attrs = {}) noexcept :
			value{ Enumerator::ValueOf(e) }, attrs{ std::move(attrs) }{}

		Enumerator::Value value;
		std::unordered_map<size_t, std::any> attrs;
	};
}
