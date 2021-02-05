#pragma once

#include <array>

#define OBJECT_PTR_DEFINE_OPERATOR_T(type, op, name)                                  \
template<typename Arg>                                                                \
SharedObject type::operator op (Arg&& rhs) const {                                    \
    return ADMInvoke(StrIDRegistry::MetaID::operator_##name, std::forward<Arg>(rhs)); \
}

#define OBJECT_PTR_DEFINE_CONTAINER_T(type, name)                                      \
template<typename Arg>                                                                 \
SharedObject type::name (Arg&& rhs) const {                                            \
    return ADMInvoke(StrIDRegistry::MetaID::container_##name, std::forward<Arg>(rhs)); \
}

#define OBJECT_PTR_DEFINE_CONTAINER_VARS_T(type, name)                                      \
template<typename... Args>                                                                  \
SharedObject type::name (Args&&... args) const {                                            \
    return ADMInvoke(StrIDRegistry::MetaID::container_##name, std::forward<Args>(args)...); \
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
	constexpr TypeID ArgID(const std::remove_const_t<std::remove_reference_t<T>>& arg) noexcept {
		using U = std::remove_cvref_t<T>;
		if constexpr (std::is_same_v<U, ObjectPtr> || std::is_same_v<U, SharedObject>)
			return ObjectPtr{ arg }.AddLValueReference().GetTypeID();
		else
			return TypeID_of<T>;
	}

	template<typename T>
	constexpr void* ArgPtr(const T& arg) noexcept {
		if constexpr (std::is_same_v<T, ObjectPtr> || std::is_same_v<T, SharedObject>)
			return arg.GetPtr();
		else
			return const_cast<void*>(static_cast<const void*>(&arg));
	}
}

namespace Ubpa::UDRefl {
	//
	// ObjectPtr
	//////////////////

	inline ObjectPtr::operator bool() const noexcept {
		if (ptr) {
			if (ID.Is<bool>())
				return As<bool>();
			else {
				auto rst = IsInvocable(StrIDRegistry::MetaID::operator_bool);
				if (rst.success) {
					assert(rst.result_desc.typeID == TypeID_of<bool>);
					return Invoke<bool>(StrIDRegistry::MetaID::operator_bool);
				}
				else
					return true;
			}
		}
		else
			return false;
	}

	template<typename... Args>
	InvocableResult ObjectPtr::IsInvocable(StrID methodID) const {
		constexpr std::array argTypeIDs = { TypeID_of<Args>... };
		return IsInvocable(methodID, std::span<const TypeID>{argTypeIDs});
	}

	template<typename T>
	T ObjectPtr::InvokeRet(StrID methodID, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
		if constexpr (!std::is_void_v<T>) {
			using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
			std::uint8_t result_buffer[sizeof(U)];
			InvokeResult result = Invoke(methodID, result_buffer, argTypeIDs, argptr_buffer);
			assert(result.resultID == TypeID_of<T>);
			return result.Move<T>(result_buffer);
		}
		else
			Invoke(methodID, (void*)nullptr, argTypeIDs, argptr_buffer);
	}

	template<typename... Args>
	InvokeResult ObjectPtr::InvokeArgs(StrID methodID, void* result_buffer, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypeIDs = { TypeID_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return Invoke(methodID, result_buffer, std::span<const TypeID>{ argTypeIDs }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return Invoke(methodID, result_buffer);
	}

	template<typename T, typename... Args>
	T ObjectPtr::Invoke(StrID methodID, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypeIDs = { TypeID_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return InvokeRet<T>(methodID, std::span<const TypeID>{ argTypeIDs }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return InvokeRet<T>(methodID);
	}

	template<typename... Args>
	SharedObject ObjectPtr::MInvoke(
		StrID methodID,
		std::pmr::memory_resource* rst_rsrc,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypeIDs = { TypeID_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return MInvoke(methodID, std::span<const TypeID>{ argTypeIDs }, static_cast<ArgPtrBuffer>(argptr_buffer.data()), rst_rsrc);
		}
		else
			return MInvoke(methodID, std::span<const TypeID>{}, static_cast<ArgPtrBuffer>(nullptr), rst_rsrc);
	}

	template<typename... Args>
	SharedObject ObjectPtr::DMInvoke(
		StrID methodID,
		Args&&... args) const
	{
		return MInvoke(methodID, std::pmr::get_default_resource(), std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	T ObjectPtr::AInvoke(StrID methodID, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { details::ArgID<decltype(args)>(args)... };
			const std::array argptr_buffer{ details::ArgPtr(args)... };
			return InvokeRet<T>(methodID, std::span<const TypeID>{ argTypeIDs }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return InvokeRet<T>(methodID);
	}

	template<typename... Args>
	SharedObject ObjectPtr::AMInvoke(
		StrID methodID,
		std::pmr::memory_resource* rst_rsrc,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { details::ArgID<decltype(args)>(args)... };
			const std::array argptr_buffer{ details::ArgPtr(args)... };
			return MInvoke(methodID, std::span<const TypeID>{ argTypeIDs }, static_cast<ArgPtrBuffer>(argptr_buffer.data()), rst_rsrc);
		}
		else
			return MInvoke(methodID, std::span<const TypeID>{}, static_cast<ArgPtrBuffer>(nullptr), rst_rsrc);
	}

	template<typename... Args>
	SharedObject ObjectPtr::ADMInvoke(
		StrID methodID,
		Args&&... args) const
	{
		return AMInvoke(methodID, std::pmr::get_default_resource(), std::forward<Args>(args)...);
	}
	
	OBJECT_PTR_DEFINE_OPERATOR_T(ObjectPtr, +, add)
	OBJECT_PTR_DEFINE_OPERATOR_T(ObjectPtr, -, sub)
	OBJECT_PTR_DEFINE_OPERATOR_T(ObjectPtr, *, mul)
	OBJECT_PTR_DEFINE_OPERATOR_T(ObjectPtr, /, div)
	OBJECT_PTR_DEFINE_OPERATOR_T(ObjectPtr, %, mod)
	OBJECT_PTR_DEFINE_OPERATOR_T(ObjectPtr, &, band)
	OBJECT_PTR_DEFINE_OPERATOR_T(ObjectPtr, |, bor)
	OBJECT_PTR_DEFINE_OPERATOR_T(ObjectPtr, ^, bxor)

	OBJECT_PTR_DEFINE_OPERATOR_T(ObjectPtr, [], subscript)
	OBJECT_PTR_DEFINE_OPERATOR_T(ObjectPtr, ->*, member_of_pointer)

	template<typename... Args>
	SharedObject ObjectPtr::operator()(Args&&... args) const {
		return DMInvoke(StrIDRegistry::MetaID::operator_call, std::forward<Args>(args)...);
	}

	template<typename T>
	SharedObject ObjectPtr::operator<<(T&& in) const {
		return ADMInvoke(StrIDRegistry::MetaID::operator_lshift, std::forward<T>(in));
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
	
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, count)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, find)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, lower_bound)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, upper_bound)
	OBJECT_PTR_DEFINE_CONTAINER_T(ObjectPtr, equal_range)
}

template<>
struct std::hash<Ubpa::UDRefl::ObjectPtr> {
	std::size_t operator()(const Ubpa::UDRefl::ObjectPtr& obj) const noexcept {
		return obj.GetTypeID().GetValue() ^ std::hash<const void*>()(obj.GetPtr());
	}
};

template<>
struct std::hash<Ubpa::UDRefl::SharedObject> {
	std::size_t operator()(const Ubpa::UDRefl::SharedObject& obj) const noexcept {
		return obj.GetTypeID().GetValue() ^ std::hash<const void*>()(obj.GetPtr());
	}
};

namespace std {
	inline void swap(Ubpa::UDRefl::SharedObject& left, Ubpa::UDRefl::SharedObject& right) noexcept {
		left.Swap(right);
	}
}

template<typename T>
struct Ubpa::UDRefl::IsObjectOrPtr {
private:
	using U = std::remove_cvref_t<T>;
public:
	static constexpr bool value =
		std::is_same_v<U, ObjectPtr>
		|| std::is_same_v<U, SharedObject>;
};

namespace Ubpa::UDRefl {
	inline bool operator== (const ObjectPtr& lhs, const ObjectPtr& rhs) {
    	return static_cast<bool>(lhs.ADMInvoke(StrIDRegistry::MetaID::operator_eq, rhs))
		|| static_cast<bool>(rhs.ADMInvoke(StrIDRegistry::MetaID::operator_eq, lhs));
	}

	inline bool operator!= (const ObjectPtr& lhs, const ObjectPtr& rhs) {
    	return static_cast<bool>(lhs.ADMInvoke(StrIDRegistry::MetaID::operator_ne, rhs))
		|| static_cast<bool>(rhs.ADMInvoke(StrIDRegistry::MetaID::operator_ne, lhs));
	}

	template<NonObjectAndPtr T>
	bool operator==(const T& lhs, ObjectPtr ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) } == ptr;
	}

	template<NonObjectAndPtr T>
	bool operator!=(const T& lhs, ObjectPtr ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) }  != ptr;
	}

	template<NonObjectAndPtr T>
	bool operator<(const T& lhs, ObjectPtr ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) }  < ptr;
	}

	template<NonObjectAndPtr T>
	bool operator>(const T& lhs, ObjectPtr ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) }  > ptr;
	}

	template<NonObjectAndPtr T>
	bool operator<=(const T& lhs, ObjectPtr ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) }  <= ptr;
	}

	template<NonObjectAndPtr T>
	bool operator>=(const T& lhs, ObjectPtr ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) }  >= ptr;
	}

	template<NonObjectAndPtr T>
	bool operator==(const T& lhs, const SharedObject& ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) }  == ptr;
	}

	template<NonObjectAndPtr T>
	bool operator!=(const T& lhs, const SharedObject& ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) }  != ptr;
	}

	template<NonObjectAndPtr T>
	bool operator<(const T& lhs, const SharedObject& ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) }  < ptr;
	}

	template<NonObjectAndPtr T>
	bool operator>(const T& lhs, const SharedObject& ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) }  > ptr;
	}

	template<NonObjectAndPtr T>
	bool operator<=(const T& lhs, const SharedObject& ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) }  <= ptr;
	}

	template<NonObjectAndPtr T>
	bool operator>=(const T& lhs, const SharedObject& ptr) {
		return ObjectPtr{ TypeID_of<T>, const_cast<T*>(&lhs) }  >= ptr;
	}

	DEFINE_OPERATOR_LSHIFT(std::ostream, ObjectPtr)
	DEFINE_OPERATOR_LSHIFT(std::ostream, SharedObject)
	DEFINE_OPERATOR_LSHIFT(std::ostringstream, ObjectPtr)
	DEFINE_OPERATOR_LSHIFT(std::ostringstream, SharedObject)
	DEFINE_OPERATOR_LSHIFT(std::ofstream, ObjectPtr)
	DEFINE_OPERATOR_LSHIFT(std::ofstream, SharedObject)
	DEFINE_OPERATOR_LSHIFT(std::iostream, ObjectPtr)
	DEFINE_OPERATOR_LSHIFT(std::iostream, SharedObject)
	DEFINE_OPERATOR_LSHIFT(std::stringstream, ObjectPtr)
	DEFINE_OPERATOR_LSHIFT(std::stringstream, SharedObject)
	DEFINE_OPERATOR_LSHIFT(std::fstream, ObjectPtr)
	DEFINE_OPERATOR_LSHIFT(std::fstream, SharedObject)

	DEFINE_OPERATOR_RSHIFT(std::istream, ObjectPtr)
	DEFINE_OPERATOR_RSHIFT(std::istringstream, ObjectPtr)
	DEFINE_OPERATOR_RSHIFT(std::ifstream, ObjectPtr)
	DEFINE_OPERATOR_RSHIFT(std::iostream, ObjectPtr)
	DEFINE_OPERATOR_RSHIFT(std::stringstream, ObjectPtr)
	DEFINE_OPERATOR_RSHIFT(std::fstream, SharedObject)
}

#undef OBJECT_PTR_DEFINE_OPERATOR_T
#undef OBJECT_PTR_DEFINE_CONTAINER_T
#undef OBJECT_PTR_DEFINE_CONTAINER_VARS_T
#undef DEFINE_OPERATOR_LSHIFT
#undef DEFINE_OPERATOR_RSHIFT
