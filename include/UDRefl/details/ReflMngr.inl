#pragma once

namespace Ubpa::UDRefl::details {
	template<typename ArgList>
	struct GenerateMethodPtr_Helper;
	template<typename... Args>
	struct GenerateMethodPtr_Helper<TypeList<Args...>> {
		static ParamList GenerateParamList(ReflMngr& mngr)
			noexcept(sizeof...(Args) == 0)
		{
			return mngr.GenerateParamList<Args...>();
		}

		template<auto funcptr, size_t... Ns>
		static constexpr auto GenerateFunction(std::index_sequence<Ns...>) noexcept {
			using FuncPtr = decltype(funcptr);
			using Traits = FuncTraits<decltype(funcptr)>;
			if constexpr (std::is_member_function_pointer_v<FuncPtr>) {
				using ObjPtr = std::conditional_t<Traits::is_const, const void*, void*>;
				constexpr auto wrapped_func = [](ObjPtr obj, ArgsView args, void* result_buffer) -> Destructor {
					assert(((args.GetParamList().GetParameters()[Ns].typeID == TypeRegistry::DirectGetID<Args>())&&...));
					assert(((args.GetParamList().GetParameters()[Ns].size == sizeof(type_buffer_decay_t<Args>))&&...));
					assert(((args.GetParamList().GetParameters()[Ns].alignment == alignof(type_buffer_decay_t<Args>))&&...));
					constexpr auto f = wrap_function<funcptr>();
					return f(obj, args.GetBuffer(), result_buffer);
				};
				return wrapped_func;
			}
			else if constexpr (is_function_pointer_v<FuncPtr>) {
				constexpr auto wrapped_func = [](ArgsView args, void* result_buffer) -> Destructor {
					assert(((args.GetParamList().GetParameters()[Ns].typeID == TypeRegistry::DirectGetID<Args>())&&...));
					assert(((args.GetParamList().GetParameters()[Ns].size == sizeof(type_buffer_decay_t<Args>))&&...));
					assert(((args.GetParamList().GetParameters()[Ns].alignment == alignof(type_buffer_decay_t<Args>))&&...));
					constexpr auto f = wrap_function<funcptr>();
					return f(args.GetBuffer(), result_buffer);
				};
				return wrapped_func;
			}
			else
				static_assert(false);
		}

		template<typename Func, size_t... Ns>
		static constexpr auto GenerateMemberFunction(Func&& func, std::index_sequence<Ns...>) noexcept {
			using Traits = WrapFuncTraits<std::decay_t<Func>>;
			using ObjPtr = std::conditional_t<Traits::is_const, const void*, void*>;
			/*constexpr*/ auto wrapped_func = [f = std::forward<Func>(func)](ObjPtr obj, ArgsView args, void* result_buffer)->Destructor {
				assert(((args.GetParamList().GetParameters()[Ns].typeID == TypeRegistry::DirectGetID<Args>())&&...));
				assert(((args.GetParamList().GetParameters()[Ns].size == sizeof(type_buffer_decay_t<Args>))&&...));
				assert(((args.GetParamList().GetParameters()[Ns].alignment == alignof(type_buffer_decay_t<Args>))&&...));
				auto wrapped_f = wrap_member_function(std::forward<Func>(f));
				return wrapped_f(obj, args.GetBuffer(), result_buffer);
			};
			return wrapped_func;
		}

		template<typename Func, size_t... Ns>
		static constexpr auto GenerateStaticFunction(Func&& func, std::index_sequence<Ns...>) noexcept {
			/*constexpr*/ auto wrapped_func = [f = std::forward<Func>(func)](ArgsView args, void* result_buffer)->Destructor {
				assert(((args.GetParamList().GetParameters()[Ns].typeID == TypeRegistry::DirectGetID<Args>())&&...));
				assert(((args.GetParamList().GetParameters()[Ns].size == sizeof(type_buffer_decay_t<Args>))&&...));
				assert(((args.GetParamList().GetParameters()[Ns].alignment == alignof(type_buffer_decay_t<Args>))&&...));
				auto wrapped_f = wrap_static_function(std::forward<Func>(f));
				return wrapped_f(args.GetBuffer(), result_buffer);
			};
			return wrapped_func;
		}
	};
};

namespace Ubpa::UDRefl {
	//
	// Generate
	/////////////

	template<typename Return>
	ResultDesc ReflMngr::GenerateResultDesc() noexcept {
		if constexpr (!std::is_void_v<Return>) {
			using T = type_buffer_decay_t<Return>;
			return {
				tregistry.GetID<Return>(),
				sizeof(T),
				alignof(T)
			};
		}
		else {
			return {
				tregistry.GetID<void>(),
				0,
				0
			};
		}
	}

	template<typename... Params>
	ParamList ReflMngr::GenerateParamList() noexcept(sizeof...(Params) == 0) {
		if constexpr (sizeof...(Params) > 0)
			return ParamList{ { {tregistry.GetID<Params>(), sizeof(type_buffer_decay_t<Params>), alignof(type_buffer_decay_t<Params>)}... } };
		else
			return {};
	}

	template<auto funcptr>
	MethodPtr ReflMngr::GenerateMethodPtr() noexcept(IsEmpty_v<FuncTraits_ArgList<decltype(funcptr)>>) {
		using FuncPtr = decltype(funcptr);
		using Traits = FuncTraits<decltype(funcptr)>;
		using ArgList = typename Traits::ArgList;
		using Return = typename Traits::Return;
		using Helper = details::GenerateMethodPtr_Helper<ArgList>;
		return {
			Helper::template GenerateFunction<funcptr>(std::make_index_sequence<Length_v<ArgList>>{}),
			GenerateResultDesc<Return>(),
			Helper::GenerateParamList(*this)
		};
	}

	template<typename Func>
	MethodPtr ReflMngr::GenerateMemberMethodPtr(Func&& func)
		noexcept(IsEmpty_v<typename details::WrapFuncTraits<std::decay_t<Func>>::ArgList>)
	{
		using Traits = details::WrapFuncTraits<std::decay_t<Func>>;
		using ArgList = typename Traits::ArgList;
		using Return = typename Traits::Return;
		using Helper = details::GenerateMethodPtr_Helper<ArgList>;
		return {
			Helper::template GenerateMemberFunction(std::forward<Func>(func), std::make_index_sequence<Length_v<ArgList>>{}),
			GenerateResultDesc<Return>(),
			Helper::GenerateParamList(*this)
		};
	}

	template<typename Func>
	MethodPtr ReflMngr::GenerateStaticMethodPtr(Func&& func) noexcept(IsEmpty_v<FuncTraits_ArgList<Func>>) {
		using Traits = FuncTraits<std::decay_t<Func>>;
		using Return = typename Traits::Return;
		using ArgList = typename Traits::ArgList;
		using Helper = details::GenerateMethodPtr_Helper<ArgList>;
		return {
			Helper::template GenerateStaticFunction(std::forward<Func>(func), std::make_index_sequence<Length_v<ArgList>>{}),
			GenerateResultDesc<Return>(),
			Helper::GenerateParamList(*this)
		};
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

	template<typename... Args>
	InvokeResult ReflMngr::InvokeArgs(TypeID typeID, NameID methodID, void* result_buffer, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return Invoke(typeID, methodID, argTypeIDs, &args_buffer, result_buffer);
		}
		else
			return Invoke(typeID, methodID, {}, nullptr, result_buffer);
	}

	template<typename... Args>
	InvokeResult ReflMngr::InvokeArgs(ConstObjectPtr obj, NameID methodID, void* result_buffer, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return Invoke(obj, methodID, argTypeIDs, &args_buffer, result_buffer);
		}
		else
			return Invoke(obj, methodID, {}, nullptr, result_buffer);
	}

	template<typename... Args>
	InvokeResult ReflMngr::InvokeArgs(ObjectPtr obj, NameID methodID, void* result_buffer, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return Invoke(obj, methodID, argTypeIDs, &args_buffer, result_buffer);
		}
		else
			return Invoke(obj, methodID, {}, nullptr, result_buffer);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(TypeID typeID, NameID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return InvokeRet<T>(typeID, methodID, argTypeIDs, &args_buffer);
		}
		else
			return InvokeRet<T>(typeID, methodID);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(ConstObjectPtr obj, NameID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return InvokeRet<T>(obj, methodID, argTypeIDs, &args_buffer);
		}
		else
			return InvokeRet<T>(obj, methodID);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(ObjectPtr obj, NameID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
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

	template<typename Obj, typename... Args>
	InvokeResult ReflMngr::InvokeArgs(NameID methodID, void* result_buffer, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return Invoke(methodID, argTypeIDs, &args_buffer, result_buffer);
		}
		else
			return Invoke(methodID, {}, nullptr, result_buffer);
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
		return IsInvocable<Args...>(TypeRegistry::DirectGetID(TypeRegistry::Meta::global), methodID);
	}

	template<typename T>
	T ReflMngr::InvokeRet(NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer) const {
		return InvokeRet<T>(TypeRegistry::DirectGetID(TypeRegistry::Meta::global), methodID, argTypeIDs, args_buffer);
	}

	template<typename... Args>
	InvokeResult ReflMngr::InvokeArgs(NameID methodID, void* result_buffer, Args... args) const {
		return InvokeArgs<Args>(TypeRegistry::DirectGetID(TypeRegistry::Meta::global), methodID, result_buffer, std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(NameID methodID, Args... args) const {
		return Invoke(TypeRegistry::DirectGetID(TypeRegistry::Meta::global), methodID, std::forward<Args>(args)...);
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
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return Construct(obj, argTypeIDs, &args_buffer);
		}
		else
			return Construct(obj);
	}

	template<typename... Args>
	ObjectPtr ReflMngr::New(TypeID typeID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return New(typeID, argTypeIDs, &args_buffer);
		}
		else
			return New(typeID);
	}

	template<typename T, typename... Args>
	ObjectPtr ReflMngr::New(Args... args) const {
		return New<Args...>(TypeRegistry::DirectGetID<T>(), std::forward<Args>(args)...);
	}

	template<typename... Args>
	SharedObject ReflMngr::MakeShared(TypeID typeID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			std::array argTypeIDs = { TypeRegistry::DirectGetID<Args>()... };
			auto args_buffer = type_buffer_decay_as_tuple<Args...>(std::forward<Args>(args)...);
			return MakeShared(typeID, argTypeIDs, &args_buffer);
		}
		else
			return MakeShared(typeID);
	}

	template<typename T, typename... Args>
	SharedObject ReflMngr::MakeShared(Args... args) const {
		return MakeShared<Args...>(TypeRegistry::DirectGetID<T>(), std::forward<Args>(args)...);
	}
}
