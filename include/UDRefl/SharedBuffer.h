#pragma once

#include <memory>
#include <functional>
#include <cassert>

namespace Ubpa::UDRefl {
	class SharedBuffer {
	public:
		//
		// Constructor
		////////////////

		constexpr SharedBuffer() noexcept = default;
		constexpr SharedBuffer(std::nullptr_t) noexcept {}
        SharedBuffer(void* ptr, void(*deleter)(void*)) : ptr{ ptr, deleter } {}
        SharedBuffer(void* ptr, std::function<void(void*)> deleter) : ptr{ ptr, deleter } {}
        SharedBuffer(void(*deleter)(void*)) : ptr{ nullptr, deleter } {}
        explicit SharedBuffer(const std::weak_ptr<void>& ptr) : ptr{ ptr } {}
        SharedBuffer(const std::shared_ptr<void>& ptr) noexcept : ptr{ ptr } {}
        SharedBuffer(std::shared_ptr<void>&& ptr) noexcept : ptr{ std::move(ptr) } {}
        SharedBuffer(SharedBuffer&& attr) noexcept : ptr{ std::move(attr.ptr) } {}
        SharedBuffer(const SharedBuffer& attr) : ptr{ attr.ptr } {} // only exception

		template<typename U>
		explicit SharedBuffer(U* ptr) : ptr{ std::shared_ptr<void>{ptr} } {}
		template<typename U, typename Deleter>
		SharedBuffer(U* ptr, Deleter&& d) : ptr{ ptr, std::forward<Deleter>(d) } {}
		template<typename Deleter>
		SharedBuffer(std::nullptr_t ptr, Deleter&& d) : ptr{ ptr, std::forward<Deleter>(d) } {}
		template<typename U, typename Deleter, typename Alloc>
		SharedBuffer(U* ptr, Deleter&& d, Alloc&& alloc) : ptr{ ptr, std::forward<Deleter>(d), std::forward<Alloc>(alloc) } {}
		template<typename Deleter, typename Alloc>
		SharedBuffer(std::nullptr_t ptr, Deleter&& d, Alloc&& alloc) : ptr{ ptr, std::forward<Deleter>(d), std::forward<Alloc>(alloc) } {}

		template<typename U>
		explicit SharedBuffer(const std::weak_ptr<U>& ptr) : ptr{ ptr } {}
		template<typename Y, typename Deleter>
		SharedBuffer(std::unique_ptr<Y, Deleter>&& r) : ptr{ std::move(r) } {}
		template<typename U>
		SharedBuffer(const std::shared_ptr<U>& ptr) noexcept : ptr{ ptr } {}
		template<typename U>
		SharedBuffer(std::shared_ptr<U>&& ptr) noexcept : ptr{ std::move(ptr) } {}

		//
		// Assign
		///////////

		SharedBuffer& operator=(SharedBuffer& rhs) noexcept {
			ptr = rhs.ptr;
			return *this;
		}

        SharedBuffer& operator=(SharedBuffer&& rhs) noexcept {
            ptr = std::move(rhs.ptr);
            return *this;
        }

        SharedBuffer& operator=(const std::shared_ptr<void>& rhs) noexcept {
            ptr = rhs;
            return *this;
        }

        SharedBuffer& operator=(std::shared_ptr<void>&& rhs) noexcept {
            ptr = std::move(rhs);
            return *this;
        }

        SharedBuffer& operator=(std::nullptr_t) noexcept {
            Reset();
            return *this;
        }

        template <typename U>
        SharedBuffer& operator=(const std::shared_ptr<U>& rhs) noexcept {
            ptr = rhs;
            return *this;
        }

        template <typename U>
        SharedBuffer& operator=(std::shared_ptr<U>&& rhs) noexcept {
            ptr = std::move(rhs);
            return *this;
        }

        template <typename U, typename Deleter>
        SharedBuffer& operator=(std::unique_ptr<U, Deleter>&& rhs) {
            ptr = std::move(rhs);
            return *this;
        }

        //
        // Cast
        /////////

        std::shared_ptr<void>&      CastToSharedPtr() & noexcept { return ptr; }
        std::shared_ptr<void>       CastToSharedPtr() && noexcept { return std::move(ptr); }
        std::shared_ptr<const void> CastToSharedPtr() const& noexcept { return std::static_pointer_cast<const void>(ptr); }

        operator std::shared_ptr<void>&     () & noexcept { return ptr; }
        operator std::shared_ptr<void>      () && noexcept { return std::move(ptr); }
        operator std::shared_ptr<const void>() const& noexcept { return std::static_pointer_cast<const void>(ptr); }

        //
        // Modifiers
        //////////////

        void Reset() noexcept { ptr.reset(); }
        template<typename U>
        void Reset(U* ptrU) { ptr.reset(ptrU); }
        template<typename U, typename Deleter>
        void Reset(U* ptrU, Deleter d) { ptr.reset(ptrU, std::move(d)); }
        template<typename U, typename Deleter, typename Alloc>
        void Reset(U* ptrU, Deleter d, Alloc alloc) { ptr.reset(ptrU, std::move(d), std::move(alloc)); }

        void Swap(SharedBuffer& rhs) noexcept { ptr.swap(rhs.ptr); }
        void Swap(std::shared_ptr<void>& rhs) noexcept { ptr.swap(rhs); }

        //
        // Observers
        //////////////

        void* Get() noexcept { return ptr.get(); }
        const void* Get() const noexcept { return ptr.get(); }

        template<typename T>
        T* AsPtr() noexcept { return reinterpret_cast<T*>(Get()); }
        template<typename T>
        const T* AsPtr() const noexcept { return const_cast<SharedBuffer*>(this)->AsPtr<T>(); }

        template<typename T>
        T& As() noexcept { assert(Get()); return *AsPtr<T>(); }
        template<typename T>
        const T& As() const noexcept { assert(Get()); return *AsPtr<T>(); }

        long UseCount() const noexcept { return ptr.use_count(); }

        explicit operator bool() const noexcept { return static_cast<bool>(ptr); }

	private:
		std::shared_ptr<void> ptr;
	};

    template <typename T, class... Args>
    SharedBuffer MakeSharedBuffer(Args&&... args) {
        if constexpr (std::is_aggregate_v<T> && !std::is_constructible_v<T, Args...>)
            return SharedBuffer{ new T{ std::forward<Args>(args)... } };
        else
            return { std::make_shared<T>(std::forward<Args>(args)...) };
    }

    template <typename T, class Alloc, class... Args>
    SharedBuffer AllocateSharedBuffer(const Alloc& alloc, Args&&... args) {
        return { std::allocate_shared<T>(alloc, std::forward<Args>(args)...) };
    }
}

template<>
struct std::hash<Ubpa::UDRefl::SharedBuffer> {
    std::size_t operator()(const Ubpa::UDRefl::SharedBuffer& obj) const noexcept {
        return std::hash<const void*>()(obj.Get());
    }
};

inline bool operator==(const Ubpa::UDRefl::SharedBuffer& left, const Ubpa::UDRefl::SharedBuffer& right) noexcept {
    return left.Get() == right.Get();
}

inline bool operator!=(const Ubpa::UDRefl::SharedBuffer& left, const Ubpa::UDRefl::SharedBuffer& right) noexcept {
    return left.Get() != right.Get();
}

inline bool operator<(const Ubpa::UDRefl::SharedBuffer& left, const Ubpa::UDRefl::SharedBuffer& right) noexcept {
    return left.Get() < right.Get();
}

inline bool operator>=(const Ubpa::UDRefl::SharedBuffer& left, const Ubpa::UDRefl::SharedBuffer& right) noexcept {
    return left.Get() >= right.Get();
}

inline bool operator>(const Ubpa::UDRefl::SharedBuffer& left, const Ubpa::UDRefl::SharedBuffer& right) noexcept {
    return left.Get() > right.Get();
}

inline bool operator<=(const Ubpa::UDRefl::SharedBuffer& left, const Ubpa::UDRefl::SharedBuffer& right) noexcept {
    return left.Get() <= right.Get();
}

template <class Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& out, const Ubpa::UDRefl::SharedBuffer& obj) {
    return out << obj.Get();
}

namespace std {
    inline void swap(Ubpa::UDRefl::SharedBuffer& left, Ubpa::UDRefl::SharedBuffer& right) noexcept {
        left.Swap(right);
    }

    inline void swap(Ubpa::UDRefl::SharedBuffer& left, shared_ptr<void>& right) noexcept {
        left.Swap(right);
    }

    inline void swap(shared_ptr<void>& left, Ubpa::UDRefl::SharedBuffer& right) noexcept {
        right.Swap(left);
    }
}
