#pragma once

#define OBJECT_VIEW_DEFINE_OPERATOR_T(op, name)                                  \
template<typename T>                                                             \
SharedObject ObjectView::operator op (T&& rhs) const {                           \
    return AInvoke(NameIDRegistry::Meta::operator_##name, std::forward<T>(rhs)); \
}

#define OBJECT_VIEW_DEFINE_META_T(prefix, name)                                  \
template<typename T>                                                             \
SharedObject ObjectView::name (T&& arg) const {                                  \
    return AInvoke(NameIDRegistry::Meta::prefix##_##name, std::forward<T>(arg)); \
}

#define OBJECT_VIEW_DEFINE_META_VARS_T(prefix, name)                                    \
template<typename... Args>                                                              \
SharedObject ObjectView::name (Args&&... args) const {                                  \
    return AInvoke(NameIDRegistry::Meta::prefix##_##name, std::forward<Args>(args)...); \
}

#define DEFINE_OPERATOR_LSHIFT(Lhs, Rhs)            \
inline Lhs& operator <<(Lhs& lhs, const Rhs& rhs) { \
	return rhs >> lhs;                              \
}

#define DEFINE_OPERATOR_RSHIFT(Lhs, Rhs)            \
inline Lhs& operator >>(Lhs& lhs, const Rhs& rhs) { \
	rhs << lhs;                                     \
	return lhs;                                     \
}

namespace Ubpa::UDRefl::details {
	template<typename T>
	constexpr Type ArgType(const std::remove_const_t<std::remove_reference_t<T>>& arg) noexcept {
		using U = std::remove_cvref_t<T>;
		if constexpr (std::is_same_v<U, ObjectView> || std::is_same_v<U, SharedObject>)
			return ObjectView{ arg }.AddLValueReferenceWeak().GetType();
		else
			return Type_of<T>;
	}

	template<typename T>
	constexpr void* ArgPtr(const T& arg) noexcept {
		if constexpr (std::is_same_v<T, ObjectView> || std::is_same_v<T, SharedObject>)
			return arg.GetPtr();
		else
			return const_cast<void*>(static_cast<const void*>(&arg));
	}
}

namespace Ubpa::UDRefl {
	template<typename... Args>
	Type ObjectView::IsInvocable(Name method_name, MethodFlag flag) const {
		constexpr Type argTypes[] = { Type_of<Args>... };
		return IsInvocable(method_name, std::span<const Type>{argTypes}, flag);
	}

	template<typename T>
	T ObjectView::BInvokeRet(Name method_name, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer, MethodFlag flag) const {
		if constexpr (!std::is_void_v<T>) {
			using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
			std::aligned_storage_t<sizeof(U), alignof(U)> result_buffer;
			Type result_type = BInvoke(method_name, static_cast<void*>(&result_buffer), argTypes, argptr_buffer, flag);
			assert(result_type.Is<T>());
			return MoveResult<T>(result_type, &result_buffer);
		}
		else
			BInvoke(method_name, (void*)nullptr, argTypes, argptr_buffer, flag);
	}

	template<typename T, typename... Args>
	T ObjectView::BInvoke(Name method_name, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr Type argTypes[] = { Type_of<decltype(args)>... };
			void* const argptr_buffer[] = { const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return BInvokeRet<T>(method_name, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer));
		}
		else
			return BInvokeRet<T>(method_name);
	}

	template<typename... Args>
	SharedObject ObjectView::MInvoke(
		Name method_name,
		std::pmr::memory_resource* rst_rsrc,
		std::pmr::memory_resource* temp_args_rsrc,
		MethodFlag flag,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			constexpr Type argTypes[] = { Type_of<decltype(args)>... };
			void* const argptr_buffer[] = { const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return MInvoke(method_name, rst_rsrc, temp_args_rsrc, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer), flag);
		}
		else
			return MInvoke(method_name, rst_rsrc, temp_args_rsrc, std::span<const Type>{}, static_cast<ArgPtrBuffer>(nullptr), flag);
	}

	template<typename... Args>
	SharedObject ObjectView::Invoke(
		Name method_name,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			constexpr Type argTypes[] = { Type_of<decltype(args)>... };
			void* const argptr_buffer[] = { const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return Invoke(method_name, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer));
		}
		else
			return Invoke(method_name);
	}

	template<typename T, typename... Args>
	T ObjectView::ABInvoke(Name method_name, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr Type argTypes[] = { Type_of<decltype(args)>... };
			void* const argptr_buffer[] = { const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return BInvokeRet<T>(method_name, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer));
		}
		else
			return BInvokeRet<T>(method_name);
	}

	template<typename... Args>
	SharedObject ObjectView::AMInvoke(
		Name method_name,
		std::pmr::memory_resource* rst_rsrc,
		std::pmr::memory_resource* temp_args_rsrc,
		MethodFlag flag,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			const Type argTypes[] = { details::ArgType<decltype(args)>(args)... };
			void* const argptr_buffer[] = { details::ArgPtr(args)... };
			return MInvoke(method_name, rst_rsrc, temp_args_rsrc, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer), flag);
		}
		else
			return MInvoke(method_name, rst_rsrc, temp_args_rsrc, std::span<const Type>{}, static_cast<ArgPtrBuffer>(nullptr), flag);
	}

	template<typename... Args>
	SharedObject ObjectView::AInvoke(
		Name method_name,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			const Type argTypes[] = { details::ArgType<decltype(args)>(args)... };
			void* const argptr_buffer[] = { details::ArgPtr(args)... };
			return Invoke(method_name, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer));
		}
		else
			return Invoke(method_name, std::span<const Type>{}, static_cast<ArgPtrBuffer>(nullptr));
	}
	
	//
	// Meta
	/////////

	OBJECT_VIEW_DEFINE_OPERATOR_T(+, add)
	OBJECT_VIEW_DEFINE_OPERATOR_T(-, sub)
	OBJECT_VIEW_DEFINE_OPERATOR_T(*, mul)
	OBJECT_VIEW_DEFINE_OPERATOR_T(/, div)
	OBJECT_VIEW_DEFINE_OPERATOR_T(%, mod)
	OBJECT_VIEW_DEFINE_OPERATOR_T(&, band)
	OBJECT_VIEW_DEFINE_OPERATOR_T(|, bor)
	OBJECT_VIEW_DEFINE_OPERATOR_T(^, bxor)

	OBJECT_VIEW_DEFINE_OPERATOR_T([], subscript)

	template<typename... Args>
	SharedObject ObjectView::operator()(Args&&... args) const {
		return Invoke(NameIDRegistry::Meta::operator_call, std::forward<Args>(args)...);
	}

	template<typename T>
	T& ObjectView::operator>>(T& out) const {
		BInvoke<void>(NameIDRegistry::Meta::operator_rshift, out);
		return out;
	}

	template<typename T>
	SharedObject ObjectView::operator<<(T&& in) const {
		return AInvoke(NameIDRegistry::Meta::operator_lshift, std::forward<T>(in));
	}

	//
	// iterator
	/////////////

	OBJECT_VIEW_DEFINE_META_T(iterator, next);
	OBJECT_VIEW_DEFINE_META_T(iterator, prev);

	//
	// container
	//////////////

	// - element access

	OBJECT_VIEW_DEFINE_META_T(container, at)

	// - modifiers

	OBJECT_VIEW_DEFINE_META_VARS_T(container, insert)
	OBJECT_VIEW_DEFINE_META_VARS_T(container, insert_or_assign)
	OBJECT_VIEW_DEFINE_META_T(container, erase)
	OBJECT_VIEW_DEFINE_META_T(container, extract)

	// - lookup
	
	OBJECT_VIEW_DEFINE_META_T(container, find)
	OBJECT_VIEW_DEFINE_META_T(container, lower_bound)
	OBJECT_VIEW_DEFINE_META_T(container, upper_bound)
	OBJECT_VIEW_DEFINE_META_T(container, equal_range)

	//
	// Inline
	///////////

	inline SharedObject ObjectView::operator+() const { return Invoke(NameIDRegistry::Meta::operator_plus); }
	inline SharedObject ObjectView::operator-() const { return Invoke(NameIDRegistry::Meta::operator_minus); }
	inline SharedObject ObjectView::operator~() const { return Invoke(NameIDRegistry::Meta::operator_bnot); }
	inline SharedObject ObjectView::operator[](std::size_t n) const { return Invoke(NameIDRegistry::Meta::operator_subscript, std::move(n)); }
	inline SharedObject ObjectView::operator*() const { return Invoke(NameIDRegistry::Meta::operator_deref); }
	inline SharedObject ObjectView::operator++() const { return Invoke(NameIDRegistry::Meta::operator_pre_inc); }
	inline SharedObject ObjectView::operator++(int) const { return Invoke(NameIDRegistry::Meta::operator_post_inc, 0); }
	inline SharedObject ObjectView::operator--() const { return Invoke(NameIDRegistry::Meta::operator_pre_dec); }
	inline SharedObject ObjectView::operator--(int) const { return Invoke(NameIDRegistry::Meta::operator_post_dec, 0); }
	inline SharedObject ObjectView::begin() const { return Invoke(NameIDRegistry::Meta::container_begin); }
	inline SharedObject ObjectView::end() const { return Invoke(NameIDRegistry::Meta::container_end); }
	inline SharedObject ObjectView::rbegin() const { return Invoke(NameIDRegistry::Meta::container_rbegin); }
	inline SharedObject ObjectView::rend() const { return Invoke(NameIDRegistry::Meta::container_rend); }
	inline SharedObject ObjectView::data() const { return Invoke(NameIDRegistry::Meta::container_data); }
	inline SharedObject ObjectView::front() const { return Invoke(NameIDRegistry::Meta::container_front); }
	inline SharedObject ObjectView::back() const { return Invoke(NameIDRegistry::Meta::container_back); }
	inline SharedObject ObjectView::next() const { return Invoke(NameIDRegistry::Meta::iterator_next); }
	inline SharedObject ObjectView::prev() const { return Invoke(NameIDRegistry::Meta::iterator_prev); }
	inline SharedObject ObjectView::cbegin() const { return Invoke(NameIDRegistry::Meta::container_cbegin); }
	inline SharedObject ObjectView::cend() const { return Invoke(NameIDRegistry::Meta::container_cend); }
	inline SharedObject ObjectView::crbegin() const { return Invoke(NameIDRegistry::Meta::container_crbegin); }
	inline SharedObject ObjectView::crend() const { return Invoke(NameIDRegistry::Meta::container_crend); }
}

template<>
struct std::hash<Ubpa::UDRefl::ObjectView> {
	std::size_t operator()(const Ubpa::UDRefl::ObjectView& obj) const noexcept {
		return obj.GetType().GetID().GetValue() ^ std::hash<const void*>()(obj.GetPtr());
	}
};

template<>
struct std::hash<Ubpa::UDRefl::SharedObject> {
	std::size_t operator()(const Ubpa::UDRefl::SharedObject& obj) const noexcept {
		return obj.GetType().GetID().GetValue() ^ std::hash<const void*>()(obj.GetPtr());
	}
};

namespace std {
	inline void swap(Ubpa::UDRefl::SharedObject& left, Ubpa::UDRefl::SharedObject& right) noexcept {
		left.Swap(right);
	}
}

namespace Ubpa::UDRefl {
	inline bool operator== (const ObjectView& lhs, const ObjectView& rhs) {
    	return static_cast<bool>(lhs.AInvoke(NameIDRegistry::Meta::operator_eq, rhs))
		|| static_cast<bool>(rhs.AInvoke(NameIDRegistry::Meta::operator_eq, lhs));
	}

	inline bool operator!= (const ObjectView& lhs, const ObjectView& rhs) {
    	return static_cast<bool>(lhs.AInvoke(NameIDRegistry::Meta::operator_ne, rhs))
		|| static_cast<bool>(rhs.AInvoke(NameIDRegistry::Meta::operator_ne, lhs));
	}

	template<NonObjectAndView T>
	bool operator==(const T& lhs, ObjectView ptr) {
		return ObjectView{ lhs } == ptr;
	}

	template<NonObjectAndView T>
	bool operator!=(const T& lhs, ObjectView ptr) {
		return ObjectView{ lhs }  != ptr;
	}

	template<NonObjectAndView T>
	bool operator<(const T& lhs, ObjectView ptr) {
		return ObjectView{ lhs }  < ptr;
	}

	template<NonObjectAndView T>
	bool operator>(const T& lhs, ObjectView ptr) {
		return ObjectView{ lhs }  > ptr;
	}

	template<NonObjectAndView T>
	bool operator<=(const T& lhs, ObjectView ptr) {
		return ObjectView{ lhs }  <= ptr;
	}

	DEFINE_OPERATOR_LSHIFT(std::ostream, ObjectView)
	DEFINE_OPERATOR_LSHIFT(std::ostringstream, ObjectView)
	DEFINE_OPERATOR_LSHIFT(std::ofstream, ObjectView)
	DEFINE_OPERATOR_LSHIFT(std::iostream, ObjectView)
	DEFINE_OPERATOR_LSHIFT(std::stringstream, ObjectView)
	DEFINE_OPERATOR_LSHIFT(std::fstream, ObjectView)

	DEFINE_OPERATOR_RSHIFT(std::istream, ObjectView)
	DEFINE_OPERATOR_RSHIFT(std::istringstream, ObjectView)
	DEFINE_OPERATOR_RSHIFT(std::ifstream, ObjectView)
	DEFINE_OPERATOR_RSHIFT(std::iostream, ObjectView)
	DEFINE_OPERATOR_RSHIFT(std::stringstream, ObjectView)
	DEFINE_OPERATOR_RSHIFT(std::fstream, ObjectView)
}

#undef OBJECT_VIEW_DEFINE_OPERATOR_T
#undef OBJECT_VIEW_DEFINE_META_T
#undef OBJECT_VIEW_DEFINE_META_VARS_T
#undef DEFINE_OPERATOR_LSHIFT
#undef DEFINE_OPERATOR_RSHIFT
