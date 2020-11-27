#pragma once

#include <cassert>

namespace Ubpa::UDRefl {
	class ConstObjectPtr {
	public:
		constexpr ConstObjectPtr() noexcept : ID{ static_cast<size_t>(-1) }, ptr{ nullptr }{}
		constexpr ConstObjectPtr(std::nullptr_t) noexcept : ConstObjectPtr{} {}
		template<typename T>
		constexpr ConstObjectPtr(size_t ID, const T* ptr) noexcept : ID{ ID }, ptr { ptr } {}
		constexpr ConstObjectPtr(size_t ID, std::nullptr_t) noexcept : ID{ ID }, ptr{ nullptr } {}

		constexpr size_t GetID() const noexcept { return ID; }
		constexpr const void* GetPtr() const noexcept { return ptr; }

		template<typename T>
		constexpr const T* AsPtr() const noexcept { return reinterpret_cast<const T*>(ptr); }
		template<typename T>
		constexpr const T& As() const noexcept { assert(ptr); return *AsPtr<T>(); }

		constexpr void Reset() noexcept { *this = ConstObjectPtr{}; }

		constexpr operator bool() const noexcept { return ptr != nullptr; }
		constexpr operator const void*() const noexcept { return ptr; }

		ConstObjectPtr& operator=(std::nullptr_t) noexcept {
			ID = static_cast<size_t>(-1);
			ptr = nullptr;
		}

	private:
		size_t ID;
		const void* ptr;
	};

	class ObjectPtr {
	public:
		constexpr ObjectPtr() noexcept : ID{ static_cast<size_t>(-1) }, ptr{ nullptr }{}
		constexpr ObjectPtr(std::nullptr_t) noexcept : ObjectPtr{} {}
		template<typename T>
		constexpr ObjectPtr(size_t ID, T* ptr) noexcept : ID{ ID }, ptr{ ptr } {}
		constexpr ObjectPtr(size_t ID, std::nullptr_t) noexcept : ID{ ID }, ptr{ nullptr } {}

		constexpr size_t GetID() const noexcept { return ID; }
		constexpr void* GetPtr() const noexcept { return ptr; }
		template<typename T>
		constexpr T* GetPtr() const noexcept { return ptr; }

		template<typename T>
		constexpr T* AsPtr() const noexcept { return reinterpret_cast<T*>(ptr); }
		template<typename T>
		constexpr T& As() const noexcept { assert(ptr); return *AsPtr<T>(); }

		constexpr void Reset() noexcept { *this = ObjectPtr{}; }

		constexpr operator bool() const noexcept { return ptr != nullptr; }
		constexpr operator void* () const noexcept { return ptr; }
		constexpr operator ConstObjectPtr() const noexcept { return { ID, ptr }; }

		ConstObjectPtr& operator=(std::nullptr_t) noexcept { Reset(); }

	private:
		size_t ID;
		void* ptr;
	};
}
