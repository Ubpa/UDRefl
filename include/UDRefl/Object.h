#pragma once

#include <cstdint>

namespace Ubpa::UDRefl {
	class Object {
	public:
		Object(size_t id, void* ptr) noexcept : id{ id }, ptr{ ptr }{}
		Object() noexcept : id{ static_cast<size_t>(-1) }, ptr{ nullptr }{}

		void* Pointer() noexcept { return ptr; }
		const void* Pointer() const noexcept { return const_cast<Object*>(this)->Pointer(); }

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

	private:
		size_t id;
		void* ptr;
	};
}
