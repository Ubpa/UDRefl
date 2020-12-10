#pragma once

#include "TypeIDRegistry.h"

#include <cassert>

namespace Ubpa::UDRefl {
	class ConstObjectPtr {
	public:
		constexpr ConstObjectPtr() noexcept : ptr{ nullptr }{}
		constexpr ConstObjectPtr(std::nullptr_t) noexcept : ConstObjectPtr{} {}
		constexpr ConstObjectPtr(TypeID ID, const void* ptr) noexcept : ID{ ID }, ptr { ptr } {}
		constexpr ConstObjectPtr(TypeID ID, std::nullptr_t) noexcept : ID{ ID }, ptr{ nullptr } {}
		explicit constexpr ConstObjectPtr(TypeID ID) noexcept : ConstObjectPtr{ID, nullptr} {}

		constexpr TypeID GetID() const noexcept { return ID; }
		constexpr const void* GetPtr() const noexcept { return ptr; }

		template<typename T>
		constexpr bool Is() const noexcept { return ID == TypeID::Of<T>(); }

		template<typename T>
		constexpr const T* AsPtr() const noexcept {
			assert(Is<T>());
			return reinterpret_cast<const T*>(ptr);
		}
		template<typename T>
		constexpr const T& As() const noexcept { assert(ptr); return *AsPtr<T>(); }

		constexpr void Reset() noexcept { ptr = nullptr; }
		constexpr void Clear() noexcept { *this = ConstObjectPtr{}; }
		ConstObjectPtr& operator=(std::nullptr_t) noexcept { Reset(); }

		constexpr operator const void* () const noexcept { return ptr; }
		constexpr operator bool() const noexcept { return ptr != nullptr; }

	private:
		TypeID ID;
		const void* ptr;
	};

	class ObjectPtr {
	public:
		constexpr ObjectPtr() noexcept : ptr{ nullptr }{}
		constexpr ObjectPtr(std::nullptr_t) noexcept : ObjectPtr{} {}
		constexpr ObjectPtr(TypeID ID, void* ptr) noexcept : ID{ ID }, ptr{ ptr } {}
		constexpr ObjectPtr(TypeID ID, std::nullptr_t) noexcept : ID{ ID }, ptr{ nullptr } {}
		explicit constexpr ObjectPtr(TypeID ID) noexcept : ObjectPtr{ ID, nullptr } {}

		constexpr TypeID GetID() const noexcept { return ID; }
		constexpr void* GetPtr() const noexcept { return ptr; }

		template<typename T>
		constexpr bool Is() const noexcept { return ID == TypeID::Of<T>(); }

		template<typename T>
		constexpr T* AsPtr() const noexcept { return reinterpret_cast<T*>(ptr); }
		template<typename T>
		constexpr T& As() const noexcept { assert(ptr); return *AsPtr<T>(); }

		constexpr void Reset() noexcept { ptr = nullptr; }
		constexpr void Clear() noexcept { *this = ObjectPtr{}; }
		ObjectPtr& operator=(std::nullptr_t) noexcept { Reset(); }

		constexpr operator void* () const noexcept { return ptr; }
		constexpr operator bool() const noexcept { return ptr != nullptr; }
		constexpr operator ConstObjectPtr() const noexcept { return {ID, ptr}; }

	private:
		TypeID ID;
		void* ptr;
	};
}
