#pragma once

namespace Ubpa::UDRefl {
	//
	// Generate
	/////////////

	template<typename... Params>
	ParamList ReflMngr::GenerateParamList(Span<NameID, sizeof...(Params)> paramNameIDs) const
		noexcept(sizeof...(Params) == 0)
	{
		if constexpr (sizeof...(Params) > 0) {
			std::vector<Parameter> params{ {tregistry.GetID<Params>(), sizeof(Params), alignof(Params)}... };
			if (!paramNameIDs.empty()) {
				for (size_t i = 0; i < sizeof...(Params); i++)
					params[i].nameID = paramNameIDs[i];
			}
			return ParamList{ std::move(params) };
		}
		else
			return {};
	}

	//
	// Invoke
	///////////

	template<typename... Args>
	bool ReflMngr::IsStaticInvocable(TypeID typeID, NameID methodID) const noexcept {
		std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
		return IsStaticInvocable(typeID, methodID, argTypeIDs);
	}

	template<typename... Args>
	bool ReflMngr::IsConstInvocable(TypeID typeID, NameID methodID) const noexcept {
		std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
		return IsConstInvocable(typeID, methodID, argTypeIDs);
	}

	template<typename... Args>
	bool ReflMngr::IsInvocable(TypeID typeID, NameID methodID) const noexcept {
		std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
		return IsInvocable(typeID, methodID, argTypeIDs);
	}

	template<typename T>
	T ReflMngr::InvokeRet(TypeID typeID, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer) const {
		std::uint8_t result_buffer[sizeof(T)];
		auto result = Invoke(typeID, methodID, argTypeIDs, args_buffer, result_buffer);
		assert(result.resultID == TypeRegistry::DirectGetID<T>());
		return result.Move<T>(result_buffer);
	}

	template<typename T>
	T ReflMngr::InvokeRet(ConstObjectPtr obj, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer) const {
		std::uint8_t result_buffer[sizeof(T)];
		auto result = Invoke(obj, methodID, argTypeIDs, args_buffer, result_buffer);
		assert(result.resultID == TypeRegistry::DirectGetID<T>());
		return result.Move<T>(result_buffer);
	}

	template<typename T>
	T ReflMngr::InvokeRet(ObjectPtr obj, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer) const {
		std::uint8_t result_buffer[sizeof(T)];
		auto result = Invoke(obj, methodID, argTypeIDs, args_buffer, result_buffer);
		assert(result.resultID == TypeRegistry::DirectGetID<T>());
		return result.Move<T>(result_buffer);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(TypeID typeID, NameID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = remove_lref_as_tuple_buffer<Args...>(std::forward<Args>(args)...);
			return InvokeRet<T>(typeID, methodID, argTypeIDs, &args_buffer);
		}
		else
			return InvokeRet<T>(typeID, methodID);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(ConstObjectPtr obj, NameID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = remove_lref_as_tuple_buffer<Args...>(std::forward<Args>(args)...);
			return InvokeRet<T>(obj, methodID, argTypeIDs, &args_buffer);
		}
		else
			return InvokeRet<T>(obj, methodID);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(ObjectPtr obj, NameID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = remove_lref_as_tuple_buffer<Args...>(std::forward<Args>(args)...);
			return InvokeRet<T>(obj, methodID, argTypeIDs, &args_buffer);
		}
		else
			return InvokeRet<T>(obj, methodID);
	}

	template<typename Obj, typename... Args>
	bool ReflMngr::IsStaticInvocable(NameID methodID) const noexcept {
		return IsStaticInvocable<Args...>(TypeRegistry::DirectGetID<Obj>(), methodID);
	}

	template<typename Obj, typename... Args>
	bool ReflMngr::IsConstInvocable(NameID methodID) const noexcept {
		return IsConstInvocable<Args...>(TypeRegistry::DirectGetID<Obj>(), methodID);
	}

	template<typename Obj, typename... Args>
	bool ReflMngr::IsInvocable(NameID methodID) const noexcept {
		return IsInvocable<Args...>(TypeRegistry::DirectGetID<Obj>(), methodID);
	}

	template<typename Obj, typename T>
	T ReflMngr::InvokeRet(NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer) const {
		return InvokeRet(TypeRegistry::DirectGetID<Obj>(), methodID, argTypeIDs, args_buffer);
	}

	template<typename Obj, typename T, typename... Args>
	T ReflMngr::Invoke(NameID methodID, Args... args) const {
		return Invoke(TypeRegistry::DirectGetID<Obj>(), methodID, std::forward<Args>(args));
	}

	//
	// Meta
	/////////

	template<typename... Args>
	bool ReflMngr::IsInvocable(NameID methodID) const noexcept {
		std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
		return IsInvocable(methodID, argTypeIDs);
	}

	template<typename T>
	T ReflMngr::InvokeRet(NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer) const {
		std::uint8_t result_buffer[sizeof(T)];
		auto result = Invoke(methodID, argTypeIDs, args_buffer, result_buffer);
		assert(result.resultID == TypeRegistry::DirectGetID<T>());
		return result.Move<T>(result_buffer);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(NameID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = remove_lref_as_tuple_buffer<Args...>(std::forward<Args>(args)...);
			return InvokeRet<T>(methodID, argTypeIDs, &args_buffer);
		}
		else
			return InvokeRet<T>(methodID);
	}

	template<typename... Args>
	bool ReflMngr::IsConstructible(TypeID typeID) const noexcept {
		std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
		return IsConstructible(typeID, argTypeIDs);
	}

	template<typename... Args>
	bool ReflMngr::Construct(ObjectPtr obj, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = remove_lref_as_tuple_buffer<Args...>(std::forward<Args>(args)...);
			return Construct(obj, argTypeIDs, &args_buffer);
		}
		else
			return Construct(obj);
	}

	template<typename... Args>
	ObjectPtr ReflMngr::New(TypeID typeID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = remove_lref_as_tuple_buffer<Args...>(std::forward<Args>(args)...);
			return New(typeID, argTypeIDs, &args_buffer);
		}
		else
			return New(typeID);
	}
}
