#pragma once

#include <array>

#define OBJECT_VIEW_DEFINE_OPERATOR_T(type, op, name)                                  \
template<typename Arg>                                                                \
SharedObject type::operator op (Arg&& rhs) const {                                    \
    return ADMInvoke(NameIDRegistry::Meta::operator_##name, std::forward<Arg>(rhs)); \
}

#define OBJECT_VIEW_DEFINE_CONTAINER_T(type, name)                                      \
template<typename Arg>                                                                 \
SharedObject type::name (Arg&& rhs) const {                                            \
    return ADMInvoke(NameIDRegistry::Meta::container_##name, std::forward<Arg>(rhs)); \
}

#define OBJECT_VIEW_DEFINE_CONTAINER_VARS_T(type, name)                                      \
template<typename... Args>                                                                  \
SharedObject type::name (Args&&... args) const {                                            \
    return ADMInvoke(NameIDRegistry::Meta::container_##name, std::forward<Args>(args)...); \
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
				auto rst = IsInvocable(NameIDRegistry::Meta::operator_bool);
				if (rst.success) {
					assert(rst.result_desc.type.Is<bool>());
					return Invoke<bool>(NameIDRegistry::Meta::operator_bool);
				}
				else
					return true;
			}
		}
		else
			return false;
	}

	template<typename... Args>
	InvocableResult ObjectView::IsInvocable(Name method_name, FuncFlag flag) const {
		constexpr std::array argTypes = { Type_of<Args>... };
		return IsInvocable(method_name, std::span<const Type>{argTypes}, flag);
	}

	template<typename T>
	T ObjectView::InvokeRet(Name method_name, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer, FuncFlag flag) const {
		if constexpr (!std::is_void_v<T>) {
			using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
			std::uint8_t result_buffer[sizeof(U)];
			InvokeResult result = Invoke(method_name, result_buffer, argTypes, argptr_buffer, flag);
			assert(result.type.Is<T>());
			return result.Move<T>(result_buffer);
		}
		else
			Invoke(method_name, (void*)nullptr, argTypes, argptr_buffer, flag);
	}

	template<typename... Args>
	InvokeResult ObjectView::InvokeArgs(Name method_name, void* result_buffer, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return Invoke(method_name, result_buffer, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return Invoke(method_name, result_buffer);
	}

	template<typename T, typename... Args>
	T ObjectView::Invoke(Name method_name, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return InvokeRet<T>(method_name, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return InvokeRet<T>(method_name);
	}

	template<typename... Args>
	SharedObject ObjectView::MInvoke(
		Name method_name,
		std::pmr::memory_resource* rst_rsrc,
		FuncFlag flag,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return MInvoke(method_name, rst_rsrc, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()), flag);
		}
		else
			return MInvoke(method_name, rst_rsrc, std::span<const Type>{}, static_cast<ArgPtrBuffer>(nullptr), flag);
	}

	template<typename... Args>
	SharedObject ObjectView::DMInvoke(
		Name method_name,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return DMInvoke(method_name, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return DMInvoke(method_name);
	}

	template<typename T, typename... Args>
	T ObjectView::AInvoke(Name method_name, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypes = { details::ArgType<decltype(args)>(args)... };
			const std::array argptr_buffer{ details::ArgPtr(args)... };
			return InvokeRet<T>(method_name, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return InvokeRet<T>(method_name);
	}

	template<typename... Args>
	SharedObject ObjectView::AMInvoke(
		Name method_name,
		std::pmr::memory_resource* rst_rsrc,
		FuncFlag flag,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypes = { details::ArgType<decltype(args)>(args)... };
			const std::array argptr_buffer{ details::ArgPtr(args)... };
			return MInvoke(method_name, rst_rsrc, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()), flag);
		}
		else
			return MInvoke(method_name, rst_rsrc, std::span<const Type>{}, static_cast<ArgPtrBuffer>(nullptr), flag);
	}

	template<typename... Args>
	SharedObject ObjectView::ADMInvoke(
		Name method_name,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypes = { details::ArgType<decltype(args)>(args)... };
			const std::array argptr_buffer{ details::ArgPtr(args)... };
			return DMInvoke(method_name, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return DMInvoke(method_name, std::span<const Type>{}, static_cast<ArgPtrBuffer>(nullptr));
	}
	
	OBJECT_VIEW_DEFINE_OPERATOR_T(ObjectView, +, add)
	OBJECT_VIEW_DEFINE_OPERATOR_T(ObjectView, -, sub)
	OBJECT_VIEW_DEFINE_OPERATOR_T(ObjectView, *, mul)
	OBJECT_VIEW_DEFINE_OPERATOR_T(ObjectView, /, div)
	OBJECT_VIEW_DEFINE_OPERATOR_T(ObjectView, %, mod)
	OBJECT_VIEW_DEFINE_OPERATOR_T(ObjectView, &, band)
	OBJECT_VIEW_DEFINE_OPERATOR_T(ObjectView, |, bor)
	OBJECT_VIEW_DEFINE_OPERATOR_T(ObjectView, ^, bxor)

	OBJECT_VIEW_DEFINE_OPERATOR_T(ObjectView, [], subscript)
	OBJECT_VIEW_DEFINE_OPERATOR_T(ObjectView, ->*, member_of_pointer)

	template<typename... Args>
	SharedObject ObjectView::operator()(Args&&... args) const {
		return DMInvoke(NameIDRegistry::Meta::operator_call, std::forward<Args>(args)...);
	}

	template<typename T>
	SharedObject ObjectView::operator<<(T&& in) const {
		return ADMInvoke(NameIDRegistry::Meta::operator_lshift, std::forward<T>(in));
	}

	//
	// container
	//////////////

	OBJECT_VIEW_DEFINE_CONTAINER_VARS_T(ObjectView, assign)

	// - element access

	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, at)

	// - capacity

	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, resize)

	// - modifiers

	OBJECT_VIEW_DEFINE_CONTAINER_VARS_T(ObjectView, insert)
	OBJECT_VIEW_DEFINE_CONTAINER_VARS_T(ObjectView, insert_or_assign)
	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, erase)
	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, push_front)
	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, push_back)
	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, swap)
	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, merge)
	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, extract)

	// - lookup
	
	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, count)
	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, find)
	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, lower_bound)
	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, upper_bound)
	OBJECT_VIEW_DEFINE_CONTAINER_T(ObjectView, equal_range)
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

template<typename T>
struct Ubpa::UDRefl::IsObjectOrView {
private:
	using U = std::remove_cvref_t<T>;
public:
	static constexpr bool value =
		std::is_same_v<U, ObjectView>
		|| std::is_same_v<U, SharedObject>;
};

namespace Ubpa::UDRefl {
	inline bool operator== (const ObjectView& lhs, const ObjectView& rhs) {
    	return static_cast<bool>(lhs.ADMInvoke(NameIDRegistry::Meta::operator_eq, rhs))
		|| static_cast<bool>(rhs.ADMInvoke(NameIDRegistry::Meta::operator_eq, lhs));
	}

	inline bool operator!= (const ObjectView& lhs, const ObjectView& rhs) {
    	return static_cast<bool>(lhs.ADMInvoke(NameIDRegistry::Meta::operator_ne, rhs))
		|| static_cast<bool>(rhs.ADMInvoke(NameIDRegistry::Meta::operator_ne, lhs));
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
#undef OBJECT_VIEW_DEFINE_CONTAINER_T
#undef OBJECT_VIEW_DEFINE_CONTAINER_VARS_T
#undef DEFINE_OPERATOR_LSHIFT
#undef DEFINE_OPERATOR_RSHIFT
