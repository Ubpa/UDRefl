#pragma once

#define OBJECT_VIEW_DEFINE_OPERATOR_T(op, name)                                    \
template<typename Arg>                                                             \
SharedObject ObjectView::operator op (Arg&& rhs) const {                           \
    return AInvoke(NameIDRegistry::Meta::operator_##name, std::forward<Arg>(rhs)); \
}

#define OBJECT_VIEW_DEFINE_META_T(prefix, name)                                    \
template<typename Arg>                                                             \
SharedObject ObjectView::name (Arg&& rhs) const {                                  \
    return AInvoke(NameIDRegistry::Meta::prefix##_##name, std::forward<Arg>(rhs)); \
}

#define OBJECT_VIEW_DEFINE_META_RET_T(prefix, name, ret)                                 \
template<typename Arg>                                                                   \
ret ObjectView::name (Arg&& rhs) const {                                                 \
    return ABInvoke<ret>(NameIDRegistry::Meta::prefix##_##name, std::forward<Arg>(rhs)); \
}

#define OBJECT_VIEW_DEFINE_CONTAINER_VARS_T(name)                                        \
template<typename... Args>                                                               \
SharedObject ObjectView::name (Args&&... args) const {                                   \
    return AInvoke(NameIDRegistry::Meta::container_##name, std::forward<Args>(args)...); \
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
	//
	// ObjectView
	//////////////////

	inline ObjectView::operator bool() const noexcept {
		if (ptr) {
			if (type.Is<bool>())
				return As<bool>();
			else {
				if (auto rst = IsInvocable(NameIDRegistry::Meta::operator_bool)) {
					assert(rst.Is<bool>());
					return BInvoke<bool>(NameIDRegistry::Meta::operator_bool);
				}
				else
					return true;
			}
		}
		else
			return false;
	}

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
	SharedObject ObjectView::operator<<(T&& in) const {
		return AInvoke(NameIDRegistry::Meta::operator_lshift, std::forward<T>(in));
	}

	//
	// iterator
	/////////////

	template<typename Arg>
	void ObjectView::advance (Arg&& rhs) const {
		ABInvoke<void>(NameIDRegistry::Meta::iterator_advance, std::forward<Arg>(rhs));
	}
	OBJECT_VIEW_DEFINE_META_RET_T(iterator, distance, std::size_t);
	OBJECT_VIEW_DEFINE_META_T(iterator, next);
	OBJECT_VIEW_DEFINE_META_T(iterator, prev);

	//
	// container
	//////////////

	// - element access

	OBJECT_VIEW_DEFINE_META_T(container, at)

	// - capacity

	OBJECT_VIEW_DEFINE_META_RET_T(container, resize, void)

	// - modifiers

	OBJECT_VIEW_DEFINE_CONTAINER_VARS_T(insert)
	OBJECT_VIEW_DEFINE_CONTAINER_VARS_T(insert_or_assign)
	OBJECT_VIEW_DEFINE_META_T(container, erase)
	OBJECT_VIEW_DEFINE_META_RET_T(container, push_front, void)
	OBJECT_VIEW_DEFINE_META_RET_T(container, push_back, void)
	OBJECT_VIEW_DEFINE_META_RET_T(container, swap, void)
	OBJECT_VIEW_DEFINE_META_RET_T(container, merge, void)
	OBJECT_VIEW_DEFINE_META_T(container, extract)

	// - lookup
	
	OBJECT_VIEW_DEFINE_META_RET_T(container, count, std::size_t)
	OBJECT_VIEW_DEFINE_META_T(container, find)
	OBJECT_VIEW_DEFINE_META_T(container, lower_bound)
	OBJECT_VIEW_DEFINE_META_T(container, upper_bound)
	OBJECT_VIEW_DEFINE_META_T(container, equal_range)
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
#undef OBJECT_VIEW_DEFINE_CONTAINER_VARS_T
#undef DEFINE_OPERATOR_LSHIFT
#undef DEFINE_OPERATOR_RSHIFT
