#pragma once

#include <any>
#include <cassert>

namespace Ubpa::UDRefl {
	struct AnyWrapper {
		template<typename T>
		AnyWrapper(T data) : data{ data } {}
		AnyWrapper() = default;

		std::any data;

		bool HasValue() const noexcept {
			return data.has_value();
		}

		const std::type_info& Type() const noexcept {
			return data.type();
		}

		template<typename T>
		bool TypeIs() const noexcept {
			return Type() == typeid(T);
		}

		template<typename T>
		bool operator==(const T& t) const {
			return HasValue() && TypeIs<T>() && Cast<T>() == t;
		}

		template<typename T>
		T& Cast() {
			assert(HasValue() && TypeIs<T>());
			return std::any_cast<T&>(data);
		}

		template<typename T>
		const T& Cast() const {
			return const_cast<AnyWrapper*>(this)->Cast<T>();
		}

		template<typename T>
		T* CastIf() {
			return std::any_cast<T>(&data);
		}

		template<typename T>
		const T* CastIf() const {
			return const_cast<AnyWrapper*>(this)->CastIf<T>();
		}
	};
}
