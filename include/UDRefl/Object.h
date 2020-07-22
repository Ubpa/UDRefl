#pragma once

#include "Export.h"

#include <cstdint>

namespace Ubpa::UDRefl {
	struct TypeInfo;

	class UDREFL_DESC Object {
	public:
		Object(size_t id, void* ptr) noexcept : id{ id }, ptr{ ptr }{}
		Object() noexcept : id{ static_cast<size_t>(-1) }, ptr{ nullptr }{}

		void* Pointer() noexcept { return ptr; }
		const void* Pointer() const noexcept { return const_cast<Object*>(this)->Pointer(); }

		template<typename T>
		T* As() noexcept { return reinterpret_cast<T*>(ptr); }
		template<typename T>
		const T* As() const noexcept { return const_cast<Object*>(this)->As<T>(); }

		const size_t& ID() const noexcept { return id; }

		// non-static
		template<typename T>
		T& Var(size_t offset) noexcept {
			return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(ptr) + offset);
		}

		template<typename T>
		const T& Var(size_t offset) const noexcept {
			return const_cast<Object*>(this)->Var<T>(offset);
		}

		bool Valid() const noexcept {
			return id != static_cast<size_t>(-1) && ptr != nullptr;
		}

		TypeInfo* GetTypeInfo() const;

	private:
		size_t id;
		void* ptr;
	};
}
