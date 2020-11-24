#pragma once

#include <cassert>

namespace Ubpa::UDRefl {
	class ConstObjectPtr {
	public:
		constexpr ConstObjectPtr() noexcept : ID{ static_cast<size_t>(-1) }, ptr{ nullptr }{}
		constexpr ConstObjectPtr(std::nullptr_t) noexcept : ConstObjectPtr{} {}
		template<typename T>
		constexpr ConstObjectPtr(size_t ID, const T* ptr) noexcept : ID{ ID }, ptr { ptr } {}

		size_t GetID() const noexcept { return ID; }
		const void* GetPtr() const noexcept { return ptr; }

		template<typename T>
		T* AsPtr() const noexcept { return reinterpret_cast<const T*>(ptr); }
		template<typename T>
		const T& As() const noexcept { assert(*this); return *AsPtr<T>(); }
		
		constexpr void Reset() noexcept { *this = ConstObjectPtr{}; }

		constexpr operator bool() const noexcept { return ptr != nullptr; }

		ConstObjectPtr& operator=(std::nullptr_t) noexcept { Reset(); }

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

		size_t GetID() const noexcept { return ID; }
		void* GetPtr() const noexcept { return ptr; }
		template<typename T>
		T* GetPtr() const noexcept { return ptr; }

		template<typename T>
		T* AsPtr() const noexcept { return reinterpret_cast<T*>(ptr); }
		template<typename T>
		T& As() const noexcept { assert(*this); return *AsPtr<T>(); }

		constexpr void Reset() noexcept { *this = ObjectPtr{}; }

		constexpr operator bool() const noexcept { return ptr != nullptr; }
		constexpr operator ConstObjectPtr() const noexcept { return { ID, ptr }; }

		ConstObjectPtr& operator=(std::nullptr_t) noexcept { Reset(); }

	private:
		size_t ID;
		void* ptr;
	};
}
