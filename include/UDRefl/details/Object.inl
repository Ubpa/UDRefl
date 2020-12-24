#pragma once

#include <array>

#define OBJECT_PTR_DEFINE_OPERATOR(type, op, name)                                         \
template<typename Arg>                                                                     \
SharedObject type::operator op (Arg rhs) const {                                           \
    return ADMInvoke<Arg>(StrIDRegistry::MetaID::operator_##name, std::forward<Arg>(rhs)); \
}

#define OBJECT_PTR_DEFINE_CONTAINER_T(type, name)                                           \
template<typename Arg>                                                                      \
SharedObject type::name (Arg rhs) const {                                                   \
    return ADMInvoke<Arg>(StrIDRegistry::MetaID::container_##name, std::forward<Arg>(rhs)); \
}

#define OBJECT_PTR_DEFINE_CONTAINER_VARS_T(type, name)                                               \
template<typename... Args>                                                                           \
SharedObject type::name (Args... args) const {                                                       \
    return ADMInvoke<Args...>(StrIDRegistry::MetaID::container_##name, std::forward<Args>(args)...); \
}

#define SHARED_OBJECT_DEFINE_OPERATOR_T(type, op)                    \
template<typename Arg>                                               \
SharedObject type::operator op (Arg rhs) const {                     \
    return AsObjectPtr()->operator op <Arg>(std::forward<Arg>(rhs)); \
}

namespace Ubpa::UDRefl {
	//
	// ObjectPtrBase
	//////////////////

	template<typename... Args>
	InvocableResult ObjectPtrBase::IsInvocable(StrID methodID) const noexcept {
		std::array argTypeIDs = { TypeID::of<Args>... };
		return IsInvocable(methodID, Span<const TypeID>{argTypeIDs});
	}

	template<typename T>
	T ObjectPtrBase::InvokeRet(StrID methodID, Span<const TypeID> argTypeIDs, void* args_buffer) const {
		using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
		std::uint8_t result_buffer[sizeof(U)];
		InvokeResult result = Invoke(methodID, result_buffer, argTypeIDs, args_buffer);
		assert(result.resultID == TypeID::of<T>);
		return result.Move<T>(result_buffer);
	}

	template<typename... Args>
	InvokeResult ObjectPtrBase::InvokeArgs(StrID methodID, void* result_buffer, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return Invoke(methodID, result_buffer, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return Invoke(methodID, result_buffer);
	}

	template<typename T, typename... Args>
	T ObjectPtrBase::Invoke(StrID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return InvokeRet<T>(methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return InvokeRet<T>(methodID);
	}

	template<typename... Args>
	SharedObject ObjectPtrBase::MInvoke(
		StrID methodID,
		std::pmr::memory_resource* rst_rsrc,
		Args... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return MInvoke(methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()), rst_rsrc);
		}
		else
			return MInvoke(methodID, Span<const TypeID>{}, static_cast<void*>(nullptr), rst_rsrc);
	}

	template<typename... Args>
	SharedObject ObjectPtrBase::DMInvoke(
		StrID methodID,
		Args... args) const
	{
		return MInvoke<Args...>(methodID, std::pmr::get_default_resource(), std::forward<Args>(args)...);
	}

	template<typename... Args>
	SharedObject ObjectPtrBase::AMInvoke(
		StrID methodID,
		std::pmr::memory_resource* rst_rsrc,
		Args... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { ArgID<Args>(std::forward<Args>(args))... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(ArgPtr(args))... };
			return MInvoke(methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()), rst_rsrc);
		}
		else
			return MInvoke(methodID, Span<const TypeID>{}, static_cast<void*>(nullptr), rst_rsrc);
	}

	template<typename... Args>
	SharedObject ObjectPtrBase::ADMInvoke(
		StrID methodID,
		Args... args) const
	{
		return AMInvoke<Args...>(methodID, std::pmr::get_default_resource(), std::forward<Args>(args)...);
	}
	
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtrBase, +, add)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtrBase, -, sub)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtrBase, *, mul)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtrBase, /, div)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtrBase, %, mod)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtrBase, &, band)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtrBase, |, bor)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtrBase, ^, bxor)

	template<typename... Args>
	SharedObject ObjectPtrBase::operator()(Args... args) const {
		return DMInvoke<Args...>(StrIDRegistry::MetaID::operator_call,
			std::forward<Args>(args)...);
	}

	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtrBase, at)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtrBase, count)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtrBase, find)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtrBase, lower_bound)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtrBase, upper_bound)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtrBase, equal_range)

	//
	// ObjectPtr
	//////////////

	template<typename... Args>
	InvocableResult ObjectPtr::IsInvocable(StrID methodID) const noexcept {
		std::array argTypeIDs = { TypeID::of<Args>... };
		return IsInvocable(methodID, Span<const TypeID>{argTypeIDs});
	}

	template<typename T>
	T ObjectPtr::InvokeRet(StrID methodID, Span<const TypeID> argTypeIDs, void* args_buffer) const {\
		if constexpr (!std::is_void_v<T>) {
			using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
			std::uint8_t result_buffer[sizeof(U)];
			InvokeResult result = Invoke(methodID, result_buffer, argTypeIDs, args_buffer);
			assert(result.resultID == TypeID::of<T>);
			return result.Move<T>(result_buffer);
		}
		else
			Invoke(methodID, nullptr, argTypeIDs, args_buffer);
	}

	template<typename... Args>
	InvokeResult ObjectPtr::InvokeArgs(StrID methodID, void* result_buffer, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return Invoke(methodID, result_buffer, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return Invoke(methodID, result_buffer);
	}

	template<typename T, typename... Args>
	T ObjectPtr::Invoke(StrID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return InvokeRet<T>(methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return InvokeRet<T>(methodID);
	}

	template<typename... Args>
	SharedObject ObjectPtr::MInvoke(
		StrID methodID,
		std::pmr::memory_resource* rst_rsrc,
		Args... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return MInvoke(methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()), rst_rsrc);
		}
		else
			return MInvoke(methodID, Span<const TypeID>{}, static_cast<void*>(nullptr), rst_rsrc);
	}

	template<typename... Args>
	SharedObject ObjectPtr::DMInvoke(
		StrID methodID,
		Args... args) const
	{
		return MInvoke<Args...>(methodID, std::pmr::get_default_resource(), std::forward<Args>(args)...);
	}

	template<typename... Args>
	SharedObject ObjectPtr::AMInvoke(
		StrID methodID,
		std::pmr::memory_resource* rst_rsrc,
		Args... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { ArgID<Args>(std::forward<Args>(args))... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(ArgPtr(args))... };
			return MInvoke(methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()), rst_rsrc);
		}
		else
			return MInvoke(methodID, Span<const TypeID>{}, static_cast<void*>(nullptr), rst_rsrc);
	}

	template<typename... Args>
	SharedObject ObjectPtr::ADMInvoke(
		StrID methodID,
		Args... args) const
	{
		return AMInvoke<Args...>(methodID, std::pmr::get_default_resource(), std::forward<Args>(args)...);
	}
		
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, =, assign)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, +=, assign_add)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, -=, assign_sub)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, *=, assign_mul)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, /=, assign_div)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, %=, assign_mod)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, &=, assign_band)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, |=, assign_bor)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, ^=, assign_bxor)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, <<=, assign_lshift)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, >>=, assign_rshift)

	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, [], subscript)
	OBJECT_PTR_DEFINE_OPERATOR(ObjectPtr, ->*, member_of_pointer)

	template<typename... Args>
	SharedObject ObjectPtr::operator()(Args... args) const {
		return DMInvoke<Args...>(StrIDRegistry::MetaID::operator_call,
			std::forward<Args>(args)...);
	}

	template<typename T>
	SharedObject ObjectPtr::operator<<(const T& in) const {
		return ADMInvoke<const T&>(StrIDRegistry::MetaID::operator_lshift, in);
	}

	//
	// container
	//////////////

	OBJECT_PTR_DEFINE_CONTAINER_VARS_T(ObjectPtr, assign)

	// - element access

	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, at)

	// - capacity

	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, resize)

	// - modifiers

	OBJECT_PTR_DEFINE_CONTAINER_VARS_T(ObjectPtr, insert)
	OBJECT_PTR_DEFINE_CONTAINER_VARS_T(ObjectPtr, insert_or_assign)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, erase)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, push_front)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, push_back)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, swap)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, merge)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, extract)

	// - lookup

	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, find)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, lower_bound)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, upper_bound)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, equal_range)

	//
	// SharedObjectBase
	/////////////////////

	SHARED_OBJECT_DEFINE_OPERATOR_T(SharedObjectBase, +)
	SHARED_OBJECT_DEFINE_OPERATOR_T(SharedObjectBase, -)
	SHARED_OBJECT_DEFINE_OPERATOR_T(SharedObjectBase, *)
	SHARED_OBJECT_DEFINE_OPERATOR_T(SharedObjectBase, /)
	SHARED_OBJECT_DEFINE_OPERATOR_T(SharedObjectBase, %)
	SHARED_OBJECT_DEFINE_OPERATOR_T(SharedObjectBase, &)
	SHARED_OBJECT_DEFINE_OPERATOR_T(SharedObjectBase, |)
	SHARED_OBJECT_DEFINE_OPERATOR_T(SharedObjectBase, ^)

	SHARED_OBJECT_DEFINE_OPERATOR_T(SharedObjectBase, [])
	SHARED_OBJECT_DEFINE_OPERATOR_T(SharedObjectBase, ->*)

	template<typename... Args>
	SharedObject SharedObjectBase::operator()(Args... args) const {
		return AsObjectPtr()->operator() < Args... > (std::forward<Args>(args)...);
	}
}

template<>
struct std::hash<Ubpa::UDRefl::ObjectPtr> {
	std::size_t operator()(const Ubpa::UDRefl::ObjectPtr& obj) const noexcept {
		return obj.GetID().GetValue() ^ std::hash<const void*>()(obj.GetPtr());
	}
};

template<>
struct std::hash<Ubpa::UDRefl::ConstObjectPtr> {
	std::size_t operator()(const Ubpa::UDRefl::ConstObjectPtr& obj) const noexcept {
		return obj.GetID().GetValue() ^ std::hash<const void*>()(obj.GetPtr());
	}
};

template<>
struct std::hash<Ubpa::UDRefl::SharedObject> {
	std::size_t operator()(const Ubpa::UDRefl::SharedObject& obj) const noexcept {
		return obj.GetID().GetValue() ^ std::hash<const void*>()(obj.GetPtr());
	}
};

template<>
struct std::hash<Ubpa::UDRefl::SharedConstObject> {
	std::size_t operator()(const Ubpa::UDRefl::SharedConstObject& obj) const noexcept {
		return obj.GetID().GetValue() ^ std::hash<const void*>()(obj.GetPtr());
	}
};

namespace std {
	inline void swap(Ubpa::UDRefl::SharedObject& left, Ubpa::UDRefl::SharedObject& right) noexcept {
		left.Swap(right);
	}
	inline void swap(Ubpa::UDRefl::SharedConstObject& left, Ubpa::UDRefl::SharedConstObject& right) noexcept {
		left.Swap(right);
	}
}

template<typename T>
struct Ubpa::UDRefl::IsObjectOrPtr {
private:
	using U = std::remove_cv_t<T>;
public:
	static constexpr bool value =
		std::is_same_v<T, ObjectPtr>
		|| std::is_same_v<T, ConstObjectPtr>
		|| std::is_same_v<T, SharedObject>
		|| std::is_same_v<T, SharedConstObject>;
};

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator==(const T& lhs, Ubpa::UDRefl::ConstObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) == Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator!=(const T& lhs, Ubpa::UDRefl::ConstObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) != Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator<(const T& lhs, Ubpa::UDRefl::ConstObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) < Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator>(const T& lhs, Ubpa::UDRefl::ConstObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) > Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator<=(const T& lhs, Ubpa::UDRefl::ConstObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) <= Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator>=(const T& lhs, Ubpa::UDRefl::ConstObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) >= Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
T& operator<<(T& lhs, Ubpa::UDRefl::ConstObjectPtr ptr) {
	return ptr >> lhs;
}

//template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
//Ubpa::UDRefl::SharedObject operator>>(const T& lhs, Ubpa::UDRefl::ConstObjectPtr ptr) {
//	return ptr << lhs;
//}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator==(const T& lhs, Ubpa::UDRefl::ObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) == ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator!=(const T& lhs, Ubpa::UDRefl::ObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) != ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator<(const T& lhs, Ubpa::UDRefl::ObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) < ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator>(const T& lhs, Ubpa::UDRefl::ObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) > ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator<=(const T& lhs, Ubpa::UDRefl::ObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) <= ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator>=(const T& lhs, Ubpa::UDRefl::ObjectPtr ptr) {
	return Ubpa::UDRefl::Ptr(lhs) >= ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
T& operator<<(T& lhs, Ubpa::UDRefl::ObjectPtr ptr) {
	return ptr >> lhs;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
Ubpa::UDRefl::SharedObject operator>>(const T& lhs, Ubpa::UDRefl::ObjectPtr ptr) {
	return ptr << lhs;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator==(const T& lhs, const Ubpa::UDRefl::SharedConstObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) == Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator!=(const T& lhs, const Ubpa::UDRefl::SharedConstObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) != Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator<(const T& lhs, const Ubpa::UDRefl::SharedConstObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) < Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator>(const T& lhs, const Ubpa::UDRefl::SharedConstObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) > Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator<=(const T& lhs, const Ubpa::UDRefl::SharedConstObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) <= Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator>=(const T& lhs, const Ubpa::UDRefl::SharedConstObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) >= Ubpa::UDRefl::ConstCast(ptr);
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
T& operator<<(T& lhs, const Ubpa::UDRefl::SharedConstObject& ptr) {
	return ptr >> lhs;
}

//template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
//Ubpa::UDRefl::SharedObject operator>>(const T& lhs, const Ubpa::UDRefl::SharedConstObject& ptr) {
//	return ptr << lhs;
//}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator==(const T& lhs, const Ubpa::UDRefl::SharedObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) == ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator!=(const T& lhs, const Ubpa::UDRefl::SharedObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) != ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator<(const T& lhs, const Ubpa::UDRefl::SharedObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) < ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator>(const T& lhs, const Ubpa::UDRefl::SharedObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) > ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator<=(const T& lhs, const Ubpa::UDRefl::SharedObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) <= ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
bool operator>=(const T& lhs, const Ubpa::UDRefl::SharedObject& ptr) {
	return Ubpa::UDRefl::Ptr(lhs) >= ptr;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
T& operator<<(T& lhs, const Ubpa::UDRefl::SharedObject& ptr) {
	return ptr >> lhs;
}

template<typename T, std::enable_if_t<!Ubpa::UDRefl::IsObjectOrPtr_v<T>, int> = 0>
Ubpa::UDRefl::SharedObject operator>>(const T& lhs, const Ubpa::UDRefl::SharedObject& ptr) {
	return ptr << lhs;
}

#undef OBJECT_PTR_DEFINE_OPERATOR
#undef OBJECT_PTR_DEFINE_CONTAINER_T
#undef OBJECT_PTR_DEFINE_CONTAINER_VARS_T
#undef SHARED_OBJECT_DEFINE_OPERATOR_T
