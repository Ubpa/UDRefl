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
				//     -     | T | T & | const T & | T&& | const T&& |
				//       T   | - |  0  |     0     |  1  |     0     |
				//       T & | 0 |  -  |     0     |  0  |     0     |
				// const T & | 1 |  1  |     -     |  1  |     1     |
				//       T&& | 1 |  0  |     0     |  -  |     0     |
				// const T&& | 1 |  0  |     0     |  1  |     -     |

				// because rhs(arg)'s ID maybe have no name in the registry
				// so we use type_name_add_*_hash(...) to avoid it

				auto lhs = ReflMngr::Instance().tregistry.Nameof(params[i]);
#ifndef NDEBUG
				auto rhs = ReflMngr::Instance().tregistry.Nameof(typeIDs[i]);
#endif // !NDEBUG

				assert(!type_name_is_const(lhs) && !type_name_is_volatile(lhs));
				if (type_name_is_rvalue_reference(lhs)) { // &&{T} or &&{const{T}}
					auto unref_lhs = type_name_remove_reference(lhs); // T or const{T}
					assert(!type_name_is_volatile(unref_lhs));
					auto raw_lhs = type_name_remove_const(unref_lhs); // T
					if (TypeID{ raw_lhs } != typeIDs[i]) {
						if (!type_name_is_const(unref_lhs))
							return false;

						if (type_name_add_rvalue_reference_hash(raw_lhs) != typeIDs[i].GetValue())
							return false;
					}
				}
				else if (type_name_is_lvalue_reference(lhs)) { // &{T} or &{const{T}}
					auto unref_lhs = type_name_remove_reference(lhs); // T or const{T}
					if (!type_name_is_const(unref_lhs))
						return false;

					if (type_name_add_rvalue_reference_hash(unref_lhs) != typeIDs[i].GetValue()) {
						auto raw_lhs = type_name_remove_const(unref_lhs); // T

						if (TypeID{ raw_lhs } != typeIDs[i]
							&& type_name_add_lvalue_reference_hash(raw_lhs) != typeIDs[i].GetValue()
							&& type_name_add_rvalue_reference_hash(raw_lhs) != typeIDs[i].GetValue())
							return false;
					}
				}
				else { // T
					if (type_name_add_rvalue_reference_hash(lhs) != typeIDs[i].GetValue())
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
			static_assert(always_false<Func>);
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
			static_assert(always_false<Func>);
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
			static_assert(always_false<Func>);
	}, func);
};
