#pragma once

#include <array>
#include <iostream>
#include <sstream>
#include <fstream>

namespace Ubpa::UDRefl::details {
	template<typename ArgList>
	struct GenerateMethodPtr_Helper;
	template<typename... Args>
	struct GenerateMethodPtr_Helper<TypeList<Args...>> {
		static ParamList GenerateParamList()
			noexcept(sizeof...(Args) == 0)
		{
			return ReflMngr::GenerateParamList<Args...>();
		}

		template<auto funcptr, size_t... Ns>
		static constexpr auto GenerateFunction(std::index_sequence<Ns...>) noexcept {
			using FuncPtr = decltype(funcptr);
			using Traits = FuncTraits<decltype(funcptr)>;
			if constexpr (std::is_member_function_pointer_v<FuncPtr>) {
				using MaybeConstVoidPtr = std::conditional_t<Traits::is_const, const void*, void*>;
				constexpr auto wrapped_func = [](MaybeConstVoidPtr obj, void* result_buffer, ArgsView args) -> Destructor {
					assert(((args.GetParamList()[Ns] == Type_of<Args>)&&...));
					constexpr auto f = wrap_function<funcptr>();
					return f(obj, result_buffer, args.GetBuffer());
				};
				constexpr auto decayed_wrapped_func = DecayLambda(wrapped_func);
				return decayed_wrapped_func;
			}
			else if constexpr (is_function_pointer_v<FuncPtr>) {
				constexpr auto wrapped_func = [](void* result_buffer, ArgsView args) -> Destructor {
					assert(((args.GetParamList()[Ns] == Type_of<Args>)&&...));
					constexpr auto f = wrap_function<funcptr>();
					return f(result_buffer, args.GetBuffer());
				};
				constexpr auto decayed_wrapped_func = DecayLambda(wrapped_func);
				return decayed_wrapped_func;
			}
			else
				static_assert(always_false<FuncPtr>);
		}

		template<typename Func, size_t... Ns>
		static /*constexpr*/ auto GenerateMemberFunction(Func&& func, std::index_sequence<Ns...>) noexcept {
			using Traits = WrapFuncTraits<std::decay_t<Func>>;
			using MaybeConstVoidPtr = std::conditional_t<Traits::is_const, const void*, void*>;
			/*constexpr*/ auto wrapped_func =
				[wrapped_f = wrap_member_function(std::forward<Func>(func))](MaybeConstVoidPtr obj, void* result_buffer, ArgsView args) mutable -> Destructor {
					assert(((args.GetParamList()[Ns] == Type_of<Args>)&&...));
					return wrapped_f(obj, result_buffer, args.GetBuffer());
				};

			return std::function{ wrapped_func };
		}

		template<typename Func, size_t... Ns>
		static /*constexpr*/ auto GenerateStaticFunction(Func&& func, std::index_sequence<Ns...>) noexcept {
			/*constexpr*/ auto wrapped_func =
				[wrapped_f = wrap_static_function(std::forward<Func>(func))](void* result_buffer, ArgsView args) mutable ->Destructor {
					assert(((args.GetParamList()[Ns] == Type_of<Args>)&&...));
					return wrapped_f(result_buffer, args.GetBuffer());
				};
			return std::function{ wrapped_func };
		}
	};

	template<typename Rst, std::size_t TargetIdx, typename T>
	Rst runtime_get_impl(T&& obj, std::size_t i) {
		using U = std::remove_cvref_t<T>;
		if constexpr (TargetIdx == std::tuple_size_v<U>)
			return nullptr;
		else {
			if (i == TargetIdx)
				return ObjectView{ std::get<TargetIdx>(std::forward<T>(obj)) };
			else
				return runtime_get_impl<Rst, TargetIdx + 1>(std::forward<T>(obj), i);
		}
	}

	template<typename Rst, typename T>
	Rst runtime_get(T&& obj, std::size_t i) {
		using U = std::remove_cvref_t<T>;
		assert(i < std::tuple_size_v<U>);
		return runtime_get_impl<Rst, 0>(std::forward<T>(obj), i);
	}

	template<typename T, std::size_t... Ns>
	void register_tuple_elements(ReflMngr& mngr, std::index_sequence<Ns...>) {
		(mngr.RegisterType<std::tuple_element_t<Ns, T>>(), ...);
	}

	template<typename T>
	struct TypeAutoRegister_Default {
		static void run(ReflMngr& mngr) {
			if constexpr (std::is_pointer_v<T>) {
				mngr.RegisterType<std::remove_pointer_t<T>>();
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign, [](const T& obj, const std::nullptr_t&) -> decltype(auto) { return static_cast<T>(nullptr); });
			}

			if constexpr (is_valid_v<operator_plus, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_plus, [](const T& lhs) { return +lhs; });
			if constexpr (is_valid_v<operator_minus, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_minus, [](const T& lhs) {
				if constexpr (std::is_signed_v<T>)
					return -lhs;
			});

			if constexpr (is_valid_v<operator_bool, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_bool, [](const T& obj) -> decltype(auto) { return static_cast<bool>(obj); });

			if constexpr (is_valid_v<operator_add, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_add, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs + rhs; });
			if constexpr (is_valid_v<operator_sub, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_sub, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs - rhs; });
			if constexpr (is_valid_v<operator_mul, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_mul, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs * rhs; });
			if constexpr (is_valid_v<operator_div, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_div, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs / rhs; });
			if constexpr (is_valid_v<operator_mod, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_mod, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs % rhs; });

			if constexpr (is_valid_v<operator_bnot, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_bnot, [](const T& lhs) -> decltype(auto) { return ~lhs; });
			if constexpr (is_valid_v<operator_band, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_band, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs & rhs; });
			if constexpr (is_valid_v<operator_bor, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_bor, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs & rhs; });
			if constexpr (is_valid_v<operator_bxor, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_bxor, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs & rhs; });
			if constexpr (is_valid_v<operator_lshift, T, std::size_t>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_lshift, [](const T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs << rhs; });
			if constexpr (is_valid_v<operator_rshift, T, std::size_t>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_rshift, [](const T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs >> rhs; });

			if constexpr (is_valid_v<operator_lshift, T, std::istream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_lshift, [](T& lhs, std::istream& rhs) -> decltype(auto) { return rhs >> lhs; });
			if constexpr (is_valid_v<operator_lshift, T, std::istringstream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_lshift, [](T& lhs, std::istringstream& rhs) -> decltype(auto) { return rhs >> lhs; });
			if constexpr (is_valid_v<operator_lshift, T, std::ifstream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_lshift, [](T& lhs, std::ifstream& rhs) -> decltype(auto) { return rhs >> lhs; });
			if constexpr (is_valid_v<operator_lshift, T, std::iostream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_lshift, [](T& lhs, std::iostream& rhs) -> decltype(auto) { return rhs >> lhs; });
			if constexpr (is_valid_v<operator_lshift, T, std::stringstream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_lshift, [](T& lhs, std::stringstream& rhs) -> decltype(auto) { return rhs >> lhs; });
			if constexpr (is_valid_v<operator_lshift, T, std::fstream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_lshift, [](T& lhs, std::fstream& rhs) -> decltype(auto) { return rhs >> lhs; });

			if constexpr (is_valid_v<operator_rshift, T, std::ostream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_rshift, [](const T& lhs, std::ostream& rhs) -> decltype(auto) { return rhs << lhs; });
			if constexpr (is_valid_v<operator_rshift, T, std::ostringstream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_rshift, [](const T& lhs, std::ostringstream& rhs) -> decltype(auto) { return rhs << lhs; });
			if constexpr (is_valid_v<operator_rshift, T, std::ofstream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_rshift, [](const T& lhs, std::ofstream& rhs) -> decltype(auto) { return rhs << lhs; });
			if constexpr (is_valid_v<operator_rshift, T, std::iostream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_rshift, [](const T& lhs, std::iostream& rhs) -> decltype(auto) { return rhs << lhs; });
			if constexpr (is_valid_v<operator_rshift, T, std::stringstream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_rshift, [](const T& lhs, std::stringstream& rhs) -> decltype(auto) { return rhs << lhs; });
			if constexpr (is_valid_v<operator_rshift, T, std::fstream&>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_rshift, [](const T& lhs, std::fstream& rhs) -> decltype(auto) { return rhs << lhs; });

			if constexpr (is_valid_v<operator_pre_inc, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_pre_inc, [](T& lhs) -> decltype(auto) { return ++lhs; });
			if constexpr (is_valid_v<operator_post_inc, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_post_inc, [](T& lhs, int) -> decltype(auto) { return lhs++; });
			if constexpr (is_valid_v<operator_pre_dec, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_pre_dec, [](T& lhs) -> decltype(auto) { return --lhs; });
			if constexpr (is_valid_v<operator_post_dec, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_post_dec, [](T& lhs, int) -> decltype(auto) { return lhs--; });

			if constexpr (is_valid_v<operator_assign_copy, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs = rhs; });
			if constexpr (is_valid_v<operator_assign_move, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign, [](T& lhs, T&& rhs) -> decltype(auto) { return lhs = std::move(rhs); });
			if constexpr (is_valid_v<operator_assign_add, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign_add, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs += rhs; });
			if constexpr (is_valid_v<operator_assign_sub, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign_sub, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs -= rhs; });
			if constexpr (is_valid_v<operator_assign_mul, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign_mul, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs *= rhs; });
			if constexpr (is_valid_v<operator_assign_div, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign_div, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs /= rhs; });
			if constexpr (is_valid_v<operator_assign_mod, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign_mod, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs %= rhs; });
			if constexpr (is_valid_v<operator_assign_band, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign_band, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs &= rhs; });
			if constexpr (is_valid_v<operator_assign_bor, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign_bor, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs |= rhs; });
			if constexpr (is_valid_v<operator_assign_bxor, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign_bxor, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs ^= rhs; });
			if constexpr (is_valid_v<operator_assign_lshift, T, std::size_t>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign_lshift, [](T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs <<= rhs; });
			if constexpr (is_valid_v<operator_assign_rshift, T, std::size_t>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assign_rshift, [](T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs >>= rhs; });

			if constexpr (is_valid_v<operator_eq, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_eq, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs == rhs; });
			if constexpr (is_valid_v<operator_ne, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_ne, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs != rhs; });
			if constexpr (is_valid_v<operator_lt, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_lt, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs < rhs; });
			if constexpr (is_valid_v<operator_gt, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_gt, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs > rhs; });
			if constexpr (is_valid_v<operator_le, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_le, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs <= rhs; });
			if constexpr (is_valid_v<operator_ge, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_ge, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs >= rhs; });

			if constexpr (is_valid_v<operator_subscript, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_subscript, [](T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs[rhs]; });
			if constexpr (is_valid_v<operator_subscript_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_subscript, [](const T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs[rhs]; });
			if constexpr (is_valid_v<operator_deref, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_deref, [](T& lhs) -> decltype(auto) { return *lhs; });
			if constexpr (is_valid_v<operator_deref_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_deref, [](const T& lhs) -> decltype(auto) { return *lhs; });
			/*if constexpr (is_valid_v<operator_ref, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_ref, [](T& lhs) { return &lhs; });
			if constexpr (is_valid_v<operator_ref_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_ref, [](const T& lhs) { return &lhs; });*/
			/*if constexpr (is_valid_v<operator_member, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_member, [](T& lhs) { return lhs.operator->(); });
			if constexpr (is_valid_v<operator_member_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_member, [](const T& lhs) { return lhs.operator->(); });*/

			// iterator

			if constexpr (is_iterator_v<T>) {
				if constexpr (is_valid_v<operator_add, T, std::size_t>)
					mngr.AddMemberMethod(NameIDRegistry::Meta::operator_add, [](const T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs + rhs; });
				if constexpr (is_valid_v<operator_sub, T, std::size_t>)
					mngr.AddMemberMethod(NameIDRegistry::Meta::operator_sub, [](const T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs - rhs; });
			}

			// pair

			if constexpr (is_valid_v<pair_first_type, T>)
				mngr.RegisterType<pair_first_type<T>>();
			if constexpr (is_valid_v<pair_second_type, T>)
				mngr.RegisterType<pair_second_type<T>>();
			if constexpr (is_valid_v<pair_first, T>)
				mngr.AddField<&T::first>("first");
			if constexpr (is_valid_v<pair_second, T>)
				mngr.AddField<&T::second>("second");

			// tuple

			if constexpr (is_valid_v<tuple_size, T>) {
				mngr.AddStaticMethod(Type_of<T>, NameIDRegistry::Meta::tuple_size, []() { return std::tuple_size_v<T>; });
				mngr.AddMemberMethod(NameIDRegistry::Meta::tuple_get, [](T& t, const std::size_t& i) { return runtime_get<ObjectView>(t, i); });
				mngr.AddMemberMethod(NameIDRegistry::Meta::tuple_get, [](const T& t, const std::size_t& i) { return runtime_get<ObjectView>(t, i); });
				if constexpr (!IsArray_v<T>)
					register_tuple_elements<T>(mngr, std::make_index_sequence<std::tuple_size_v<T>>{});
			}

			// container

			// - iterator

			if constexpr (is_valid_v<container_begin, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_begin, [](T& lhs) -> decltype(auto) { return std::begin(lhs); });
			if constexpr (is_valid_v<container_begin_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_begin, [](const T& lhs) -> decltype(auto) { return std::begin(lhs); });
			if constexpr (is_valid_v<container_cbegin, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_cbegin, [](const T& lhs) -> decltype(auto) { return std::cbegin(lhs); });
			if constexpr (is_valid_v<container_end, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_end, [](T& lhs) -> decltype(auto) { return std::end(lhs); });
			if constexpr (is_valid_v<container_end_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_end, [](const T& lhs) -> decltype(auto) { return std::end(lhs); });
			if constexpr (is_valid_v<container_cend, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_cend, [](const T& lhs) -> decltype(auto) { return std::cend(lhs); });

			if constexpr (is_valid_v<container_rbegin, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_rbegin, [](T& lhs) -> decltype(auto) { return std::rbegin(lhs); });
			if constexpr (is_valid_v<container_rbegin_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_rbegin, [](const T& lhs) -> decltype(auto) { return std::rbegin(lhs); });
			if constexpr (is_valid_v<container_crbegin, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_crbegin, [](const T& lhs) -> decltype(auto) { return std::rbegin(lhs); });
			if constexpr (is_valid_v<container_rend, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_rend, [](T& lhs) -> decltype(auto) { return std::rend(lhs); });
			if constexpr (is_valid_v<container_rend_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_rend, [](const T& lhs) -> decltype(auto) { return std::rend(lhs); });
			if constexpr (is_valid_v<container_crend, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_crend, [](const T& lhs) -> decltype(auto) { return std::crend(lhs); });

			// - element access

			if constexpr (is_valid_v<container_at, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_at, [](T& lhs, const std::size_t& n) -> decltype(auto) { return lhs.at(n); });
			if constexpr (is_valid_v<container_at_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_at, [](const T& lhs, const std::size_t& n) -> decltype(auto) { return lhs.at(n); });

			if constexpr (is_valid_v<container_at_key, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_at, [](T& lhs, const typename T::key_type& key) -> decltype(auto) { return lhs.at(key); });
			if constexpr (is_valid_v<container_at_key_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_at, [](const T& lhs, const typename T::key_type& key) -> decltype(auto) { return lhs.at(key); });

			if constexpr (is_valid_v<container_subscript_key_0, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_subscript, [](T& lhs, const typename T::node_type& key) -> decltype(auto) { return lhs[key]; });
			if constexpr (is_valid_v<container_subscript_key_1, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::operator_subscript, [](T& lhs, typename T::node_type&& key) -> decltype(auto) { return lhs[std::move(key)]; });

			if constexpr (is_valid_v<container_data, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_data, [](T& lhs) -> decltype(auto) { return std::data(lhs); });
			if constexpr (is_valid_v<container_data_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_data, [](const T& lhs) -> decltype(auto) { return std::data(lhs); });

			if constexpr (is_valid_v<container_front, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_front, [](T& lhs) -> decltype(auto) { return lhs.front(); });
			if constexpr (is_valid_v<container_front_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_front, [](const T& lhs) -> decltype(auto) { return lhs.front(); });

			if constexpr (is_valid_v<container_back, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_back, [](T& lhs) -> decltype(auto) { return lhs.back(); });
			if constexpr (is_valid_v<container_back_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_back, [](const T& lhs) -> decltype(auto) { return lhs.back(); });

			if constexpr (is_valid_v<container_empty, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_empty, [](const T& lhs) -> decltype(auto) { return std::empty(lhs); });

			if constexpr (is_valid_v<container_size, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_size, [](const T& lhs) -> decltype(auto) { return std::size(lhs); });

			/*if constexpr (is_valid_v<container_max_size, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_max_size, [](const T& lhs) -> decltype(auto) { return lhs.max_size(); });*/

			if constexpr (is_valid_v<container_resize_0, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_resize, [](T& lhs, const std::size_t& n) -> decltype(auto) { return lhs.resize(n); });

			if constexpr (is_valid_v<container_resize_1, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_resize, [](T& lhs, const std::size_t& n, const typename T::value_type& value) -> decltype(auto) { return lhs.resize(n, value); });

			if constexpr (is_valid_v<container_capacity, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_capacity, [](const T& lhs) -> decltype(auto) { return lhs.capacity(); });

			if constexpr (is_valid_v<container_bucket_count, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_bucket_count, [](const T& lhs) -> decltype(auto) { return lhs.bucket_count(); });

			if constexpr (is_valid_v<container_reserve, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_reserve, [](T& lhs, const std::size_t& n) -> decltype(auto) { return lhs.reserve(n); });

			if constexpr (is_valid_v<container_shrink_to_fit, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_shrink_to_fit, [](T& lhs) -> decltype(auto) { return lhs.shrink_to_fit(); });

			// - modifiers

			if constexpr (is_valid_v<container_clear, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_clear, [](T& lhs) -> decltype(auto) { return lhs.clear(); });

			if constexpr (is_valid_v<container_insert_0, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator iter, const typename T::value_type& value) -> decltype(auto) { return lhs.insert(iter, value); });

			if constexpr (is_valid_v<container_insert_1, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator iter, typename T::value_type&& value) -> decltype(auto) { return lhs.insert(iter, std::move(value)); });

			if constexpr (is_valid_v<container_insert_2, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator iter, const std::size_t& n, const typename T::value_type& value) -> decltype(auto) { return lhs.insert(iter, n, std::move(value)); });

			if constexpr (is_valid_v<container_insert_3, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, typename T::node_type&& value) -> decltype(auto) { return lhs.insert(std::move(value)); });

			if constexpr (is_valid_v<container_insert_4, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator iter, typename T::node_type&& value) -> decltype(auto) { return lhs.insert(iter, std::move(value)); });

			// assign (TODO)

			if constexpr (is_valid_v<container_erase_0, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, typename T::iterator iter) -> decltype(auto) { return lhs.erase(iter); });

			if constexpr (is_valid_v<container_erase_1, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator iter) -> decltype(auto) { return lhs.erase(iter); });

			if constexpr (is_valid_v<container_erase_2, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, typename T::iterator start, typename T::iterator end) -> decltype(auto) { return lhs.erase(start, end); });

			if constexpr (is_valid_v<container_erase_3, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator start, typename T::const_iterator end) -> decltype(auto) { return lhs.erase(start, end); });

			if constexpr (is_valid_v<container_erase_4, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.erase(rhs); });

			if constexpr (is_valid_v<container_push_front_0, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_push_front, [](T& lhs, const typename T::value_type& value) -> decltype(auto) { return lhs.push_front(value); });

			if constexpr (is_valid_v<container_push_front_1, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_push_front, [](T& lhs, typename T::value_type&& value) -> decltype(auto) { return lhs.push_front(std::move(value)); });

			if constexpr (is_valid_v<container_pop_front, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_pop_front, [](T& lhs) -> decltype(auto) { return lhs.pop_front(); });

			if constexpr (is_valid_v<container_push_back_0, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_push_back, [](T& lhs, const typename T::value_type& value) -> decltype(auto) { return lhs.push_back(value); });
			if constexpr (is_valid_v<container_push_back_1, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_push_back, [](T& lhs, typename T::value_type&& value) -> decltype(auto) { return lhs.push_back(std::move(value)); });

			if constexpr (is_valid_v<container_pop_back, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_pop_back, [](T& lhs) -> decltype(auto) { return lhs.pop_back(); });

			if constexpr (is_valid_v<container_swap, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_swap, [](T& lhs, T& rhs) -> decltype(auto) { return std::swap(lhs, rhs); });

			if constexpr (is_valid_v<container_extract_0, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_extract, [](T& lhs, typename T::const_iterator iter) -> decltype(auto) { return lhs.extract(iter); });

			if constexpr (is_valid_v<container_extract_1, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_extract, [](T& lhs, const typename T::key_type& key) -> decltype(auto) { return lhs.extract(key); });

			if constexpr (is_valid_v<container_merge_0, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_merge, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs.merge(rhs); });
			if constexpr (is_valid_v<container_merge_1, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_merge, [](T& lhs, T&& rhs) -> decltype(auto) { return lhs.merge(std::move(rhs)); });

			// - lookup

			if constexpr (is_valid_v<container_count, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_count, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.count(rhs); });

			if constexpr (is_valid_v<container_find, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_find, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.find(rhs); });
			if constexpr (is_valid_v<container_find_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_find, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.find(rhs); });

			if constexpr (is_valid_v<container_lower_bound, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_lower_bound, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.lower_bound(rhs); });
			if constexpr (is_valid_v<container_lower_bound_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_lower_bound, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.lower_bound(rhs); });

			if constexpr (is_valid_v<container_upper_bound, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_upper_bound, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.upper_bound(rhs); });
			if constexpr (is_valid_v<container_upper_bound_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_upper_bound, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.upper_bound(rhs); });

			if constexpr (is_valid_v<container_equal_range, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_equal_range, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.equal_range(rhs); });
			if constexpr (is_valid_v<container_equal_range_const, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_equal_range, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.equal_range(rhs); });

			// - observers

			if constexpr (is_valid_v<container_key_comp, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_key_comp, [](const T& lhs) -> decltype(auto) { return lhs.key_comp(); });

			if constexpr (is_valid_v<container_value_comp, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_value_comp, [](const T& lhs) -> decltype(auto) { return lhs.value_comp(); });

			if constexpr (is_valid_v<container_hash_function, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_hash_function, [](const T& lhs) -> decltype(auto) { return lhs.hash_function(); });

			if constexpr (is_valid_v<container_key_eq, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_key_eq, [](const T& lhs) -> decltype(auto) { return lhs.key_eq(); });

			if constexpr (is_valid_v<container_get_allocator, T>)
				mngr.AddMemberMethod(NameIDRegistry::Meta::container_get_allocator, [](const T& lhs) -> decltype(auto) { return lhs.get_allocator(); });

			if constexpr (IsVector_v<T>)
				mngr.AddAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, ContainerType::Vector));
			else if constexpr (IsDeque_v<T>)
				mngr.AddAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, ContainerType::Deque));
			else if constexpr (IsArray_v<T>)
				mngr.AddAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, ContainerType::Array));
			else if constexpr (IsList_v<T>)
				mngr.AddAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, ContainerType::List));
			else if constexpr (IsForwardList_v<T>)
				mngr.AddAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, ContainerType::ForwardList));
			else if constexpr (IsMap_v<T>)
				mngr.AddAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, ContainerType::Map));
			else if constexpr (IsSet_v<T>)
				mngr.AddAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, ContainerType::Set));
			else if constexpr (IsUnorderedSet_v<T>)
				mngr.AddAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, ContainerType::UnorderedSet));
			else if constexpr (IsUnorderedMap_v<T>)
				mngr.AddAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, ContainerType::UnorderedMap));
			else if constexpr (IsStack_v<T>)
				mngr.AddAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, ContainerType::Stack));
			else if constexpr (IsQueue_v<T>)
				mngr.AddAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, ContainerType::Queue));

			// - type

			if constexpr (std::is_array_v<T>) {
				using value_type = std::remove_extent_t<T>;
				using pointer = value_type*;
				using const_pointer = const value_type*;
				using reverse_iterator = std::reverse_iterator<pointer>;
				using const_reverse_iterator = std::reverse_iterator<const_pointer>;
				mngr.RegisterType<value_type>();
				mngr.RegisterType<pointer>();
				mngr.RegisterType<const_pointer>();
				mngr.RegisterType<reverse_iterator>();
				mngr.RegisterType<const_reverse_iterator>();
			}
			else {
				if constexpr (is_valid_v<container_key_type, T>)
					mngr.RegisterType<typename T::key_type>();
				if constexpr (is_valid_v<container_mapped_type, T>)
					mngr.RegisterType<typename T::mapped_type>();
				if constexpr (is_valid_v<container_value_type, T>)
					mngr.RegisterType<typename T::value_type>();
				if constexpr (is_valid_v<container_allocator_type, T>) {
					using Allocator = typename T::allocator_type;
					mngr.RegisterType<Allocator>();
					mngr.AddMemberMethod("allocate", [](Allocator& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs.allocate(rhs); });
					mngr.AddMemberMethod("deallocate", [](Allocator& lhs, typename T::value_type* ptr, const std::size_t& num) -> decltype(auto) { return lhs.deallocate(ptr, num); });
				}
				if constexpr (is_valid_v<container_size_type, T>)
					mngr.RegisterType<typename T::size_type>();
				if constexpr (is_valid_v<container_difference_type, T>)
					mngr.RegisterType<typename T::difference_type>();
				if constexpr (is_valid_v<container_pointer_type, T>)
					mngr.RegisterType<typename T::pointer>();
				if constexpr (is_valid_v<container_const_pointer_type, T>)
					mngr.RegisterType<typename T::const_pointer>();
				if constexpr (is_valid_v<container_key_compare, T>)
					mngr.RegisterType<typename T::key_compare>();
				if constexpr (is_valid_v<container_value_coompare, T>)
					mngr.RegisterType<typename T::value_coompare>();
				if constexpr (is_valid_v<container_iterator, T>)
					mngr.RegisterType<typename T::iterator>();
				if constexpr (is_valid_v<container_const_iterator, T>)
					mngr.RegisterType<typename T::const_iterator>();
				if constexpr (is_valid_v<container_reverse_iterator, T>)
					mngr.RegisterType<typename T::reverse_iterator>();
				if constexpr (is_valid_v<container_const_reverse_iterator, T>)
					mngr.RegisterType<typename T::const_reverse_iterator>();
				if constexpr (is_valid_v<container_local_iterator, T>)
					mngr.RegisterType<typename T::local_iterator>();
				if constexpr (is_valid_v<container_const_local_iterator, T>)
					mngr.RegisterType<typename T::const_local_iterator>();
				if constexpr (is_valid_v<container_node_type, T>)
					mngr.RegisterType<typename T::node_type>();
				if constexpr (is_valid_v<container_insert_return_type, T>) {
					mngr.RegisterType<typename T::insert_return_type>();
					mngr.AddField<&T::allocator_type::position>("position");
					mngr.AddField<&T::allocator_type::inserted>("inserted");
					mngr.AddField<&T::allocator_type::node>("node");
				}
			}
		}
	};

	template<typename T>
	struct TypeAutoRegister : TypeAutoRegister_Default<T> {};
};

namespace Ubpa::UDRefl {
	//
	// Factory
	////////////

	template<auto field_data>
	FieldPtr ReflMngr::GenerateFieldPtr() {
		using FieldData = decltype(field_data);
		if constexpr (std::is_pointer_v<FieldData>) {
			using Value = std::remove_pointer_t<FieldData>;
			RegisterType<Value>();
			return {
				Type_of<Value>,
				ptr_const_cast(field_data)
			};
		}
		else if constexpr (std::is_member_object_pointer_v<FieldData>) {
			using Traits = member_pointer_traits<FieldData>;
			using Obj = typename Traits::object;
			using Value = typename Traits::value;

			RegisterType<Value>();
			if constexpr (has_virtual_base_v<Obj>) {
				return {
					Type_of<Value>,
					field_offsetor<field_data>()
				};
			}
			else {
				return {
					Type_of<Value>,
					field_forward_offset_value(field_data)
				};
			}
		}
		else if constexpr (std::is_enum_v<FieldData>) {
			using Value = std::remove_pointer_t<FieldData>;
			RegisterType<Value>();
			auto buffer = FieldPtr::ConvertToBuffer(field_data);
			return {
				Type_of<Value>,
				buffer
			};
		}
		else
			static_assert(always_false<FieldData>);
	}

	template<typename T>
	FieldPtr ReflMngr::GenerateFieldPtr(T&& data) {
		using RawT = std::remove_cv_t<std::remove_reference_t<T>>;
		static_assert(!std::is_same_v<RawT, std::size_t>);
		if constexpr (std::is_member_object_pointer_v<RawT>) {
			using Traits = member_pointer_traits<RawT>;
			using Obj = typename Traits::object;
			using Value = typename Traits::value;
			tregistry.Register<Value>();
			RegisterType<Value>();
			if constexpr (has_virtual_base_v<Obj>) {
				return {
					Type_of<Value>,
					field_offsetor(data)
				};
			}
			else {
				return {
					Type_of<Value>,
					field_forward_offset_value(data)
				};
			}
		}
		else if constexpr (std::is_pointer_v<RawT> && !is_function_pointer_v<RawT> && !std::is_void_v<std::remove_pointer_t<RawT>>) {
			using Value = std::remove_pointer_t<RawT>;
			tregistry.Register<Value>();
			RegisterType<Value>();
			return {
				Type_of<Value>,
				ptr_const_cast(data)
			};
		}
		else if constexpr (std::is_enum_v<RawT>) {
			tregistry.Register<RawT>();
			RegisterType<RawT>();
			auto buffer = FieldPtr::ConvertToBuffer(data);
			return {
				Type_of<const RawT>,
				buffer
			};
		}
		else {
			using Traits = FuncTraits<RawT>;

			using ArgList = typename Traits::ArgList;
			static_assert(Length_v<ArgList> == 1);
			using ObjPtr = Front_t<ArgList>;
			static_assert(std::is_pointer_v<ObjPtr>);
			using Obj = std::remove_pointer_t<ObjPtr>;
			static_assert(!std::is_const_v<Obj>);

			using ValuePtr = typename Traits::Return;
			static_assert(std::is_pointer_v<ValuePtr>);
			using Value = std::remove_pointer_t<ValuePtr>;
			static_assert(!std::is_void_v<Value> && !std::is_volatile_v<Value>);

			tregistry.Register<Value>();
			RegisterType<Value>();

			auto offsetor = [f=std::forward<T>(data)](void* obj) -> void* {
				return f(reinterpret_cast<Obj*>(obj));
			};

			return {
				Type_of<Value>,
				offsetor
			};
		}
	}

	template<typename T, typename... Args>
	FieldPtr ReflMngr::GenerateDynamicFieldPtr(Args&&... args) {
		static_assert(!std::is_reference_v<T> && !std::is_volatile_v<T>);
		using RawT = std::remove_const_t<T>;
		RegisterType<RawT>();
		AddConstructor<RawT, Args...>();
		if constexpr (FieldPtr::IsBufferable<RawT>()) {
			FieldPtr::Buffer buffer = FieldPtr::ConvertToBuffer(T{ std::forward<Args>(args)... });
			return FieldPtr{ Type_of<T>, buffer };
		}
		else
			return FieldPtr{ MakeShared(Type_of<RawT>, std::forward<Args>(args)...) };
	}

	template<typename Return>
	ResultDesc ReflMngr::GenerateResultDesc() {
		if constexpr (!std::is_void_v<Return>) {
			using U = std::conditional_t<std::is_reference_v<Return>, std::add_pointer_t<Return>, Return>;
			return {
				Type_of<Return>,
				sizeof(U),
				alignof(U)
			};
		}
		else
			return {};
	}

	template<typename... Params>
	ParamList ReflMngr::GenerateParamList() noexcept(sizeof...(Params) == 0) {
		if constexpr (sizeof...(Params) > 0) {
			static_assert(((!std::is_const_v<Params> && !std::is_volatile_v<Params>)&&...), "parameter type shouldn't be const.");
			return { Type_of<Params>... };
		}
		else
			return {};
	}

	template<auto funcptr>
	MethodPtr ReflMngr::GenerateMethodPtr() {
		using FuncPtr = decltype(funcptr);
		using Traits = FuncTraits<decltype(funcptr)>;
		using ArgList = typename Traits::ArgList;
		using Return = typename Traits::Return;
		using Helper = details::GenerateMethodPtr_Helper<ArgList>;
		return {
			Helper::template GenerateFunction<funcptr>(std::make_index_sequence<Length_v<ArgList>>{}),
			GenerateResultDesc<Return>(),
			Helper::GenerateParamList()
		};
	}

	template<typename T, typename... Args>
	MethodPtr ReflMngr::GenerateConstructorPtr() {
		return GenerateMemberMethodPtr([](T& obj, Args... args) {
			new(&obj)T{ std::forward<Args>(args)... };
		});
	}

	template<typename T>
	MethodPtr ReflMngr::GenerateDestructorPtr() {
		return GenerateMemberMethodPtr([](const T& obj) {
			if constexpr (!std::is_trivially_destructible_v<T>)
				obj.~T();
		});
	}

	template<typename Func>
	MethodPtr ReflMngr::GenerateMemberMethodPtr(Func&& func) {
		using Traits = details::WrapFuncTraits<std::decay_t<Func>>;
		using ArgList = typename Traits::ArgList;
		using Return = typename Traits::Return;
		using Helper = details::GenerateMethodPtr_Helper<ArgList>;
		return {
			Helper::template GenerateMemberFunction(std::forward<Func>(func), std::make_index_sequence<Length_v<ArgList>>{}),
			GenerateResultDesc<Return>(),
			Helper::GenerateParamList()
		};
	}

	template<typename Func>
	MethodPtr ReflMngr::GenerateStaticMethodPtr(Func&& func) {
		using Traits = FuncTraits<std::decay_t<Func>>;
		using Return = typename Traits::Return;
		using ArgList = typename Traits::ArgList;
		using Helper = details::GenerateMethodPtr_Helper<ArgList>;
		return {
			Helper::template GenerateStaticFunction(std::forward<Func>(func), std::make_index_sequence<Length_v<ArgList>>{}),
			GenerateResultDesc<Return>(),
			Helper::GenerateParamList()
		};
	}

	//
	// Modifier
	/////////////

	template<typename T>
	void ReflMngr::RegisterType() {
		static_assert(!std::is_volatile_v<T>);
		if constexpr (std::is_void_v<T>)
			return;
		else {
			if constexpr (std::is_const_v<T>)
				RegisterType<std::remove_const_t<T>>();
			else if constexpr (std::is_reference_v<T>)
				RegisterType<std::remove_cvref_t<T>>();
			else {
				auto target = typeinfos.find(Type_of<T>);
				if (target != typeinfos.end())
					return;

				typeinfos.emplace_hint(target, Type_of<T>, TypeInfo{ sizeof(T),alignof(T) });

				if constexpr (std::is_default_constructible_v<T>)
					AddConstructor<T>();
				if constexpr (std::is_copy_constructible_v<T>)
					AddConstructor<T, const T&>();
				if constexpr (std::is_move_constructible_v<T>)
					AddConstructor<T, T&&>();
				if constexpr (std::is_destructible_v<T>)
					AddDestructor<T>();

				details::TypeAutoRegister<T>::run(*this);
			}
		}
	}

	template<auto field_data>
	bool ReflMngr::AddField(Name name, AttrSet attrs) {
		using FieldData = decltype(field_data);
		if constexpr (std::is_enum_v<FieldData>) {
			return AddField(
				Type_of<std::remove_cv_t<FieldData>>,
				name,
				{ GenerateFieldPtr<field_data>(), std::move(attrs) }
			);
		}
		else if constexpr (std::is_member_object_pointer_v<FieldData>) {
			return AddField(
				Type_of<member_pointer_traits_object<FieldData>>,
				name,
				{ GenerateFieldPtr<field_data>(), std::move(attrs) }
			);
		}
		else
			static_assert(always_false<FieldData>, "if field_data is a static field, use AddField(Type, name, field_data, attrs)");
	}

	template<typename T,
		std::enable_if_t<!std::is_same_v<std::decay_t<T>, FieldInfo>, int>>
	bool ReflMngr::AddField(Name name, T&& data, AttrSet attrs) {
		using RawT = std::remove_cv_t<std::remove_reference_t<T>>;
		if constexpr (std::is_member_object_pointer_v<RawT>)
			return AddField(Type_of<member_pointer_traits_object<RawT>>, name, std::forward<T>(data), std::move(attrs));
		else if constexpr (std::is_enum_v<RawT>)
			return AddField(Type_of<RawT>, name, std::forward<T>(data), std::move(attrs));
		else {
			using Traits = FuncTraits<RawT>;

			using ArgList = typename Traits::ArgList;
			static_assert(Length_v<ArgList> == 1);
			using ObjPtr = Front_t<ArgList>;
			static_assert(std::is_pointer_v<ObjPtr>);
			using Obj = std::remove_pointer_t<ObjPtr>;

			return AddField(Type_of<Obj>, name, std::forward<T>(data), std::move(attrs));
		}
	}

	template<auto member_func_ptr>
	bool ReflMngr::AddMethod(Name name, AttrSet attrs) {
		using MemberFuncPtr = decltype(member_func_ptr);
		static_assert(std::is_member_function_pointer_v<MemberFuncPtr>);
		using Obj = member_pointer_traits_object<MemberFuncPtr>;
		return AddMethod(
			Type_of<Obj>,
			name,
			{ GenerateMethodPtr<member_func_ptr>(), std::move(attrs) }
		);
	}

	template<auto func_ptr>
	bool ReflMngr::AddMethod(Type type, Name name, AttrSet attrs) {
		using FuncPtr = decltype(func_ptr);
		static_assert(is_function_pointer_v<FuncPtr>);
		return AddMethod(
			type,
			name,
			{ GenerateMethodPtr<func_ptr>(), std::move(attrs) }
		);
	}

	template<typename T, typename... Args>
	bool ReflMngr::AddConstructor(AttrSet attrs) {
		return AddMethod(
			Type_of<T>,
			NameIDRegistry::Meta::ctor,
			{ GenerateConstructorPtr<T, Args...>() , std::move(attrs) }
		);
	}
	template<typename T>
	bool ReflMngr::AddDestructor(AttrSet attrs) {
		return AddMethod(
			Type_of<T>,
			NameIDRegistry::Meta::dtor,
			{ GenerateDestructorPtr<T>() , std::move(attrs) }
		);
	}

	template<typename Func>
	bool ReflMngr::AddMemberMethod(Name name, Func&& func, AttrSet attrs) {
		return AddMethod(
			Type_of<typename details::WrapFuncTraits<std::decay_t<Func>>::Object>,
			name,
			{ GenerateMemberMethodPtr(std::forward<Func>(func)), std::move(attrs) }
		);
	}

	template<typename Derived, typename Base>
	BaseInfo ReflMngr::GenerateBaseInfo() {
		return {
			inherit_cast_functions<Derived, Base>(),
			std::is_polymorphic_v<Base>,
			is_virtual_base_of_v<Base, Derived>
		};
	}

	template<typename Derived, typename... Bases>
	bool ReflMngr::AddBases() {
		return (AddBase(Type_of<Derived>, Type_of<Bases>, GenerateBaseInfo<Derived, Bases>()) && ...);
	}

	//
	// Invoke
	///////////

	template<typename... Args>
	InvocableResult ReflMngr::IsInvocable(Type type, Name method_name, MethodFlag flag) const {
		constexpr std::array argTypes = { Type_of<Args>... };
		return IsInvocable(type, method_name, std::span<const Type>{argTypes}, flag);
	}

	template<typename T>
	T ReflMngr::InvokeRet(Type type, Name method_name, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer, MethodFlag flag) const {
		if constexpr (!std::is_void_v<T>) {
			using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
			std::uint8_t result_buffer[sizeof(U)];
			InvokeResult result = Invoke(type, method_name, result_buffer, argTypes, argptr_buffer, flag);
			assert(result.type.Is<T>());
			return result.Move<T>(result_buffer);
		}
		else
			Invoke(type, method_name, (void*)nullptr, argTypes, argptr_buffer, flag);
	}

	template<typename T>
	T ReflMngr::InvokeRet(ObjectView obj, Name method_name, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer, MethodFlag flag) const {
		if constexpr (!std::is_void_v<T>) {
			using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
			std::uint8_t result_buffer[sizeof(U)];
			InvokeResult result = Invoke(obj, method_name, result_buffer, argTypes, argptr_buffer, flag);
			assert(result.type.Is<T>());
			return result.Move<T>(result_buffer);
		}
		else
			Invoke(obj, method_name, (void*)nullptr, argTypes, argptr_buffer, flag);
	}

	template<typename... Args>
	InvokeResult ReflMngr::InvokeArgs(Type type, Name method_name, void* result_buffer, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return Invoke(type, method_name, result_buffer, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return Invoke(type, method_name, result_buffer);
	}

	template<typename... Args>
	InvokeResult ReflMngr::InvokeArgs(ObjectView obj, Name method_name, void* result_buffer, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return Invoke(obj, method_name, result_buffer, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return Invoke(obj, method_name, result_buffer);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(Type type, Name method_name, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return InvokeRet<T>(type, method_name, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return InvokeRet<T>(type, method_name);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(ObjectView obj, Name method_name, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return InvokeRet<T>(obj, method_name, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return InvokeRet<T>(obj, method_name);
	}

	//
	// Meta
	/////////

	template<typename... Args>
	bool ReflMngr::IsConstructible(Type type) const {
		constexpr std::array argTypes = { Type_of<Args>... };
		return IsConstructible(type, std::span<const Type>{argTypes});
	}

	template<typename... Args>
	bool ReflMngr::Construct(ObjectView obj, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return Construct(obj, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return Construct(obj);
	}

	template<typename... Args>
	ObjectView ReflMngr::New(Type type, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return New(type, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return New(type);
	}

	template<typename T, typename... Args>
	ObjectView ReflMngr::NewAuto(Args... args) {
		static_assert(!std::is_const_v<T> && !std::is_volatile_v<T> && !std::is_reference_v<T>);
		RegisterType<T>();
		AddConstructor<T, Args...>();
		return New(Type_of<T>, std::forward<Args>(args)...);
	}

	template<typename... Args>
	SharedObject ReflMngr::MakeShared(Type type, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return MakeShared(type, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return MakeShared(type);
	}

	template<typename T, typename... Args>
	SharedObject ReflMngr::MakeSharedAuto(Args... args) {
		static_assert(!std::is_const_v<T> && !std::is_volatile_v<T> && !std::is_reference_v<T>);
		RegisterType<T>();
		AddConstructor<T, Args...>();
		return MakeShared(Type_of<T>, std::forward<Args>(args)...);
	}

	//
	// Memory
	///////////

	template<typename... Args>
	SharedObject ReflMngr::MInvoke(
		ObjectView obj,
		Name method_name,
		std::pmr::memory_resource* rst_rsrc,
		MethodFlag flag,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return MInvoke(obj, method_name, rst_rsrc, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()), flag);
		}
		else
			return MInvoke(obj, method_name, rst_rsrc, flag);
	}

	template<typename... Args>
	SharedObject ReflMngr::DMInvoke(
		ObjectView obj,
		Name method_name,
		Args&&... args) const
	{
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return DMInvoke(obj, method_name, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return DMInvoke(obj, method_name);
	}

	template<typename... Args>
	ObjectView ReflMngr::MNew(Type type, std::pmr::memory_resource* rsrc, Args&&... args) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr std::array argTypes = { Type_of<decltype(args)>... };
			const std::array argptr_buffer{ const_cast<void*>(reinterpret_cast<const void*>(&args))... };
			return MNew(type, rsrc, std::span<const Type>{ argTypes }, static_cast<ArgPtrBuffer>(argptr_buffer.data()));
		}
		else
			return MNew(type, rsrc);
	}
}
