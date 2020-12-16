#include <UDRefl/MethodPtr.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa::UDRefl;

bool ParamList::IsConpatibleWith(Span<const TypeID> typeIDs) const noexcept { 
	if (params.size() != typeIDs.size())
		return false;

	for (size_t i = 0; i < params.size(); i++) {
		if (params[i] != typeIDs[i]) {
			if ((params[i] != TypeID::of<ConstObjectPtr> || typeIDs[i] != TypeID::of<ObjectPtr>)
				&& (params[i] != TypeID::of<SharedConstObject> || typeIDs[i] != TypeID::of<SharedObject>))
			{
				auto lhs = ReflMngr::Instance().tregistry.Nameof(params[i]);
				auto rhs = ReflMngr::Instance().tregistry.Nameof(typeIDs[i]);
				assert(!type_name_is_const(lhs) && !type_name_is_volatile(lhs) && !type_name_is_const(rhs) && !type_name_is_volatile(rhs));
				if (type_name_is_rvalue_reference(lhs)) {
					if (type_name_is_lvalue_reference(rhs))
						return false;
					if (type_name_is_rvalue_reference(rhs)) {
						auto unref_lhs = type_name_remove_reference(lhs);
						if (!type_name_is_const(unref_lhs))
							return false;
						auto unref_rhs = type_name_remove_reference(rhs);
						assert(!type_name_is_volatile(unref_rhs));
						if (type_name_remove_const(unref_lhs) != unref_rhs)
							return false;
					}
					else {
						if (type_name_remove_cvref(lhs) != rhs)
							return false;
					}
				}
				else if (type_name_is_lvalue_reference(lhs)) {
					auto unref_lhs = type_name_remove_reference(lhs);
					if (!type_name_is_const(unref_lhs))
						return false;
					if (type_name_remove_const(unref_lhs) != type_name_remove_cvref(rhs))
						return false;
				}
				else {
					if (!type_name_is_rvalue_reference(rhs))
						return false;
					auto unref_rhs = type_name_remove_reference(rhs);
					if (type_name_is_const(unref_rhs))
						return false;
					if(lhs != unref_rhs)
						return false;
				}
			}
		}
	}

	return true;
}

bool ParamList::operator==(const ParamList& rhs) const noexcept {
	if (params.size() != rhs.params.size())
		return false;

	for (size_t i = 0; i < params.size(); i++) {
		if (params[i] != rhs.params[i])
			return false;
	}
	return true;
}

Destructor MethodPtr::Invoke(void* obj, void* result_buffer, void* args_buffer) const {
	return std::visit([=](const auto& f) {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			return f(     result_buffer, { args_buffer,paramList });
		else
			static_assert(false);
	}, func);
};

Destructor MethodPtr::Invoke(const void* obj, void* result_buffer, void* args_buffer) const {
	return std::visit([=](const auto& f)->Destructor {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>)
			return f(obj, result_buffer, { args_buffer,paramList });
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			return f(     result_buffer, { args_buffer,paramList });
		else
			static_assert(false);
	}, func);
};

Destructor MethodPtr::Invoke(void* result_buffer, void* args_buffer) const {
	return std::visit([=](const auto& f)->Destructor {
		using Func = std::decay_t<decltype(f)>;
		if constexpr (std::is_same_v<Func, std::function<MemberVariableFunction>>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, std::function<MemberConstFunction>>) {
			assert(false);
			return {};
		}
		else if constexpr (std::is_same_v<Func, std::function<StaticFunction>>)
			return f(result_buffer, { args_buffer,paramList });
		else
			static_assert(false);
	}, func);
};
