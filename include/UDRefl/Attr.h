#pragma once

#include "Object.h"

#include <memory>

namespace Ubpa::UDRefl {
	class Attr {
	public:
		//
		// Constructor
		////////////////

		constexpr Attr() noexcept = default;
		constexpr Attr(std::nullptr_t) noexcept {}
        Attr(void* ptr, void(*deleter)(void*)) : ptr{ ptr, deleter } {}
        Attr(void(*deleter)(void*)) : ptr{ nullptr, deleter } {}
        explicit Attr(const std::weak_ptr<void>& ptr) : ptr{ ptr } {}
        Attr(const std::shared_ptr<void>& ptr) noexcept : ptr{ ptr } {}
        Attr(std::shared_ptr<void>&& ptr) noexcept : ptr{ std::move(ptr) } {}
        Attr(Attr&& attr) noexcept : ptr{ std::move(attr.ptr) } {}
        Attr(const Attr& attr) : ptr{ attr.ptr } {} // only exception

		template<typename U>
		explicit Attr(U* ptr) : ptr{ std::shared_ptr<void>{ptr} } {}
		template<typename U, typename Deleter>
		Attr(U* ptr, Deleter d) : ptr{ ptr, std::move(d) } {}
		template<typename Deleter>
		Attr(std::nullptr_t ptr, Deleter d) : ptr{ ptr, std::move(d) } {}
		template<typename U, typename Deleter, typename Alloc>
		Attr(U* ptr, Deleter d, Alloc alloc) : ptr{ ptr, std::move(d), std::move(alloc) } {}
		template<typename Deleter, typename Alloc>
		Attr(std::nullptr_t ptr, Deleter d, Alloc alloc) : ptr{ ptr, std::move(d), std::move(alloc) } {}

		template<typename U>
		explicit Attr(const std::weak_ptr<U>& ptr) : ptr{ ptr } {}
		template<typename Y, typename Deleter>
		Attr(std::unique_ptr<Y, Deleter>&& r) : ptr{ std::move(r) } {}
		template<typename U>
		Attr(const std::shared_ptr<U>& ptr) noexcept : ptr{ ptr } {}
		template<typename U>
		Attr(std::shared_ptr<U>&& ptr) noexcept : ptr{ std::move(ptr) } {}

		//
		// Assign
		///////////

		Attr& operator=(Attr& rhs) noexcept {
			ptr = rhs.ptr;
			return *this;
		}

        Attr& operator=(Attr&& rhs) noexcept {
            ptr = std::move(rhs.ptr);
            return *this;
        }

        Attr& operator=(const std::shared_ptr<void>& rhs) noexcept {
            ptr = rhs;
            return *this;
        }

        Attr& operator=(std::shared_ptr<void>&& rhs) noexcept {
            ptr = std::move(rhs);
            return *this;
        }

        Attr& operator=(std::nullptr_t) noexcept {
            reset();
            return *this;
        }

        template <typename U>
        Attr& operator=(const std::shared_ptr<U>& rhs) noexcept {
            ptr = rhs;
            return *this;
        }

        template <typename U>
        Attr& operator=(std::shared_ptr<U>&& rhs) noexcept {
            ptr = std::move(rhs);
            return *this;
        }

        template <typename U, typename Deleter>
        Attr& operator=(std::unique_ptr<U, Deleter>&& rhs) {
            ptr = std::move(rhs);
            return *this;
        }

        //
        // Cast
        /////////

        std::shared_ptr<void>&      cast_to_shared_ptr() & noexcept { return ptr; }
        std::shared_ptr<void>       cast_to_shared_ptr() && noexcept { return std::move(ptr); }
        std::shared_ptr<const void> cast_to_shared_ptr() const& noexcept { return std::static_pointer_cast<const void>(ptr); }

        operator std::shared_ptr<void>&     () & noexcept { return ptr; }
        operator std::shared_ptr<void>      () && noexcept { return std::move(ptr); }
        operator std::shared_ptr<const void>() const& noexcept { return ptr; }

        //
        // Modifiers
        //////////////

        void reset() noexcept { ptr.reset(); }
        template<typename U>
        void reset(U* ptrU) { ptr.reset(ptrU); }
        template<typename U, typename Deleter>
        void reset(U* ptrU, Deleter d) { ptr.reset(ptrU, std::move(d)); }
        template<typename U, typename Deleter, typename Alloc>
        void reset(U* ptrU, Deleter d, Alloc alloc) { ptr.reset(ptrU, std::move(d), std::move(alloc)); }

        void swap(Attr& rhs) noexcept { return ptr.swap(rhs.ptr); }
        void swap(std::shared_ptr<void>& rhs) noexcept { return ptr.swap(rhs); }

        //
        // Observers
        //////////////

        void* get() noexcept { return ptr.get(); }
        const void* get() const noexcept { return ptr.get(); }

        template<typename T>
        T* as_ptr() noexcept { return reinterpret_cast<T*>(get()); }
        template<typename T>
        const T* as_ptr() const noexcept { return const_cast<Attr*>(this)->as_ptr<T>(); }

        ObjectPtr as_object(TypeID id) noexcept { return { id, get() }; }
        ConstObjectPtr as_object(TypeID id) const noexcept { return { id, get() }; }

        template<typename T>
        T& as() noexcept { assert(get()); return *as_ptr<T>(); }
        template<typename T>
        const T& as() const noexcept { assert(get()); return *as_ptr<T>(); }

        long use_count() const noexcept { return ptr.use_count(); }

        explicit operator bool() const noexcept { return static_cast<bool>(ptr); }

	private:
		std::shared_ptr<void> ptr;
	};

    //
    // Make Attr
    ////////////////

    template <typename T, class... Args>
    Attr MakeAttr(Args&&... args) {
        if constexpr (std::is_aggregate_v<T> && !std::is_constructible_v<T, Args...>)
            return Attr{ new T{ std::forward<Args>(args)... } };
        else
            return { std::make_shared<T>(std::forward<Args>(args)...) };
    }

    template <typename T, class Alloc, class... Args>
    Attr AllocateAttr(const Alloc& alloc, Args&&... args) {
        return { std::allocate_shared<T>(alloc, std::forward<Args>(args)...) };
    }
}

template<>
struct std::hash<Ubpa::UDRefl::Attr> {
    std::size_t operator()(const Ubpa::UDRefl::Attr& obj) noexcept {
        return std::hash<const void*>()(obj.get());
    }
};

inline bool operator==(const Ubpa::UDRefl::Attr& left, const Ubpa::UDRefl::Attr& right) noexcept {
    return left.get() == right.get();
}

inline bool operator!=(const Ubpa::UDRefl::Attr& left, const Ubpa::UDRefl::Attr& right) noexcept {
    return left.get() != right.get();
}

inline bool operator<(const Ubpa::UDRefl::Attr& left, const Ubpa::UDRefl::Attr& right) noexcept {
    return left.get() < right.get();
}

inline bool operator>=(const Ubpa::UDRefl::Attr& left, const Ubpa::UDRefl::Attr& right) noexcept {
    return left.get() >= right.get();
}

inline bool operator>(const Ubpa::UDRefl::Attr& left, const Ubpa::UDRefl::Attr& right) noexcept {
    return left.get() > right.get();
}

inline bool operator<=(const Ubpa::UDRefl::Attr& left, const Ubpa::UDRefl::Attr& right) noexcept {
    return left.get() <= right.get();
}

template <class Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& out, const Ubpa::UDRefl::Attr& obj) {
    return out << obj.get();
}

namespace std {
    inline void swap(Ubpa::UDRefl::Attr& left, Ubpa::UDRefl::Attr& right) noexcept {
        left.swap(right);
    }

    inline void swap(Ubpa::UDRefl::Attr& left, shared_ptr<void>& right) noexcept {
        left.swap(right);
    }

    inline void swap(shared_ptr<void>& left, Ubpa::UDRefl::Attr& right) noexcept {
        right.swap(left);
    }
}
