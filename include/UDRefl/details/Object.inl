#pragma once

#include <array>

namespace Ubpa::UDRefl {
	template<typename T>
	T InvokeResult::Move(void* result_buffer) {
		assert(result_buffer);

		if constexpr (!std::is_lvalue_reference_v<T> && std::is_default_constructible_v<std::remove_reference_t<T>>) {
			if (!success)
				return std::forward<T>(T{});
		}
		else
			assert(success);

		assert(resultID = TypeID::of<T>);

		if constexpr (std::is_lvalue_reference_v<T>) {
			using PtrT = std::add_pointer_t<std::remove_reference_t<T>>;
			assert(destructor == nullptr);
			return *buffer_as<PtrT>(result_buffer);
		}
		else {
			T rst = std::move(buffer_as<T>(result_buffer));
			if (destructor)
				destructor(result_buffer);
			return rst;
		}
	}

	template<typename... Args>
	bool ConstObjectPtr::IsInvocable(StrID methodID) const noexcept {
		std::array argTypeIDs = { TypeID::of<Args>... };
		return IsInvocable(ID, methodID, Span<TypeID>{argTypeIDs});
	}

	template<typename T>
	T ConstObjectPtr::InvokeRet(StrID methodID, Span<TypeID> argTypeIDs, void* args_buffer) const {
		std::uint8_t result_buffer[sizeof(T)];
		auto result = Invoke(methodID, result_buffer, argTypeIDs, args_buffer);
		assert(result.resultID == TypeID::of<T>);
		return result.Move<T>(result_buffer);
	}

	template<typename... Args>
	InvokeResult ConstObjectPtr::InvokeArgs(StrID methodID, void* result_buffer, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeID::of<Args>... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return Invoke(methodID, Span<TypeID>{ argTypeIDs }, static_cast<void*>(&args_buffer), result_buffer);
		}
		else
			return Invoke(methodID, {}, nullptr, result_buffer);
	}

	template<typename T, typename... Args>
	T ConstObjectPtr::Invoke(StrID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeID::of<Args>... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return InvokeRet<T>(methodID, Span<TypeID>{ argTypeIDs }, static_cast<void*>(&args_buffer));
		}
		else
			return InvokeRet<T>(methodID);
	}

	template<typename... Args>
	SharedObject ConstObjectPtr::MInvoke(
		StrID methodID,
		MemoryResourceType memory_rsrc_type,
		Args... args)
	{
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeID::of<Args>... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return MInvoke(methodID, Span<TypeID>{ argTypeIDs }, static_cast<void*>(&args_buffer), memory_rsrc_type);
		}
		else
			return MInvoke(methodID, Span<TypeID>{}, static_cast<void*>(nullptr), memory_rsrc_type);
	}

	template<typename... Args>
	SharedObject ConstObjectPtr::MonoMInvoke(
		StrID methodID,
		Args... args)
	{
		return MInvoke(methodID, MemoryResourceType::MONO, std::forward<Args>(args)...);
	}

	template<typename... Args>
	SharedObject ConstObjectPtr::SyncMInvoke(
		StrID methodID,
		Args... args)
	{
		return MInvoke(methodID, MemoryResourceType::SYNC, std::forward<Args>(args)...);
	}

	template<typename... Args>
	SharedObject ConstObjectPtr::UnsyncMInvoke(
		StrID methodID,
		Args... args)
	{
		return MInvoke(methodID, MemoryResourceType::UNSYNC, std::forward<Args>(args)...);
	}

	template<typename... Args>
	bool ObjectPtr::IsInvocable(StrID methodID) const noexcept {
		std::array argTypeIDs = { TypeID::of<Args>... };
		return IsInvocable(ID, methodID, Span<TypeID>{argTypeIDs});
	}

	template<typename T>
	T ObjectPtr::InvokeRet(StrID methodID, Span<TypeID> argTypeIDs, void* args_buffer) const {
		std::uint8_t result_buffer[sizeof(T)];
		auto result = Invoke(methodID, result_buffer, argTypeIDs, args_buffer);
		assert(result.resultID == TypeID::of<T>);
		return result.Move<T>(result_buffer);
	}

	template<typename... Args>
	InvokeResult ObjectPtr::InvokeArgs(StrID methodID, void* result_buffer, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeID::of<Args>... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return Invoke(methodID, Span<TypeID>{ argTypeIDs }, static_cast<void*>(&args_buffer), result_buffer);
		}
		else
			return Invoke(methodID, {}, nullptr, result_buffer);
	}

	template<typename T, typename... Args>
	T ObjectPtr::Invoke(StrID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeID::of<Args>... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return InvokeRet<T>(methodID, Span<TypeID>{ argTypeIDs }, static_cast<void*>(&args_buffer));
		}
		else
			return InvokeRet<T>(methodID);
	}

	template<typename... Args>
	SharedObject ObjectPtr::MInvoke(
		StrID methodID,
		MemoryResourceType memory_rsrc_type,
		Args... args)
	{
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeID::of<Args>... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return MInvoke(methodID, Span<TypeID>{ argTypeIDs }, static_cast<void*>(&args_buffer), memory_rsrc_type);
		}
		else
			return MInvoke(methodID, Span<TypeID>{}, static_cast<void*>(nullptr), memory_rsrc_type);
	}

	template<typename... Args>
	SharedObject ObjectPtr::MonoMInvoke(
		StrID methodID,
		Args... args)
	{
		return MInvoke(methodID, MemoryResourceType::MONO, std::forward<Args>(args)...);
	}

	template<typename... Args>
	SharedObject ObjectPtr::SyncMInvoke(
		StrID methodID,
		Args... args)
	{
		return MInvoke(methodID, MemoryResourceType::SYNC, std::forward<Args>(args)...);
	}

	template<typename... Args>
	SharedObject ObjectPtr::UnsyncMInvoke(
		StrID methodID,
		Args... args)
	{
		return MInvoke(methodID, MemoryResourceType::UNSYNC, std::forward<Args>(args)...);
	}
}

template<>
struct std::hash<Ubpa::UDRefl::SharedObject> {
	std::size_t operator()(const Ubpa::UDRefl::SharedObject& obj) const noexcept {
		return obj.GetID().GetValue() ^ std::hash<const void*>()(obj.GetPtr());
	}
};

inline bool operator==(const Ubpa::UDRefl::SharedObject& left, const Ubpa::UDRefl::SharedObject& right) noexcept {
	return left.GetID() == right.GetID() && left.GetPtr() == right.GetPtr();
}

inline bool operator!=(const Ubpa::UDRefl::SharedObject& left, const Ubpa::UDRefl::SharedObject& right) noexcept {
	return left.GetID() != right.GetID() || left.GetPtr() != right.GetPtr();
}

inline bool operator<(const Ubpa::UDRefl::SharedObject& left, const Ubpa::UDRefl::SharedObject& right) noexcept {
	return left.GetID() < right.GetID() || (left.GetID() == right.GetID() && left.GetPtr() < right.GetPtr());
}

inline bool operator>=(const Ubpa::UDRefl::SharedObject& left, const Ubpa::UDRefl::SharedObject& right) noexcept {
	return left.GetID() > right.GetID() || (left.GetID() == right.GetID() && left.GetPtr() >= right.GetPtr());
}

inline bool operator>(const Ubpa::UDRefl::SharedObject& left, const Ubpa::UDRefl::SharedObject& right) noexcept {
	return left.GetID() > right.GetID() || (left.GetID() == right.GetID() && left.GetPtr() > right.GetPtr());
}

inline bool operator<=(const Ubpa::UDRefl::SharedObject& left, const Ubpa::UDRefl::SharedObject& right) noexcept {
	return left.GetID() < right.GetID() || (left.GetID() == right.GetID() && left.GetPtr() <= right.GetPtr());
}

template <class Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& out, const Ubpa::UDRefl::SharedObject& obj) {
	return out << obj.GetID().GetValue() << obj.GetPtr();
}

namespace std {
	inline void swap(Ubpa::UDRefl::SharedObject& left, Ubpa::UDRefl::SharedObject& right) noexcept {
		left.Swap(right);
	}
}

