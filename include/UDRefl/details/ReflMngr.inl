#pragma once

#include <array>

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
				using MaybeConstVoidPtr = std::conditional_t<Traits::is_const, const void*, void*>;
				constexpr auto wrapped_func = [](MaybeConstVoidPtr obj, void* result_buffer, ArgsView args) -> Destructor {
					assert(((args.GetParamList().GetParameters()[Ns] == TypeID::of<Args>)&&...));
					constexpr auto f = wrap_function<funcptr>();
					return f(obj, result_buffer, args.GetBuffer());
				};
				constexpr auto decayed_wrapped_func = DecayLambda(wrapped_func);
				return decayed_wrapped_func;
			}
			else if constexpr (is_function_pointer_v<FuncPtr>) {
				constexpr auto wrapped_func = [](void* result_buffer, ArgsView args) -> Destructor {
					assert(((args.GetParamList().GetParameters()[Ns] == TypeID::of<Args>)&&...));
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
				[f = std::forward<Func>(func)](MaybeConstVoidPtr obj, void* result_buffer, ArgsView args) mutable -> Destructor {
					assert(((args.GetParamList().GetParameters()[Ns] == TypeID::of<Args>)&&...));
					auto wrapped_f = wrap_member_function(std::forward<Func>(f));
					return wrapped_f(obj, result_buffer, args.GetBuffer());
				};

			return std::function{ wrapped_func };
		}

		template<typename Func, size_t... Ns>
		static /*constexpr*/ auto GenerateStaticFunction(Func&& func, std::index_sequence<Ns...>) noexcept {
			/*constexpr*/ auto wrapped_func =
				[f = std::forward<Func>(func)](void* result_buffer, ArgsView args) mutable ->Destructor {
					assert(((args.GetParamList().GetParameters()[Ns] == TypeID::of<Args>)&&...));
					auto wrapped_f = wrap_static_function(std::forward<Func>(f));
					return wrapped_f(result_buffer, args.GetBuffer());
				};
			return std::function{ wrapped_func };
		}
	};

	template<typename T>
	struct TypeAutoRegister_Default {
		static void run(ReflMngr& mngr) {
			if constexpr (is_valid_v<operator_plus, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_plus, [](const T& lhs) { return +lhs; });
			if constexpr (is_valid_v<operator_minus, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_minus, [](const T& lhs) {
				if constexpr (std::is_signed_v<T>)
					return -lhs;
			});

			if constexpr (is_valid_v<operator_add, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_add, [](const T& lhs, const T& rhs) { return lhs + rhs; });
			if constexpr (is_valid_v<operator_sub, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_sub, [](const T& lhs, const T& rhs) { return lhs - rhs; });
			if constexpr (is_valid_v<operator_mul, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_mul, [](const T& lhs, const T& rhs) { return lhs * rhs; });
			if constexpr (is_valid_v<operator_div, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_div, [](const T& lhs, const T& rhs) { return lhs / rhs; });
			if constexpr (is_valid_v<operator_mod, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_mod, [](const T& lhs, const T& rhs) { return lhs % rhs; });

			if constexpr (is_valid_v<operator_bnot, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_bnot, [](const T& lhs) { return ~lhs; });
			if constexpr (is_valid_v<operator_band, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_band, [](const T& lhs, const T& rhs) { return lhs & rhs; });
			if constexpr (is_valid_v<operator_bor, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_bor, [](const T& lhs, const T& rhs) { return lhs & rhs; });
			if constexpr (is_valid_v<operator_bxor, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_bxor, [](const T& lhs, const T& rhs) { return lhs & rhs; });
			if constexpr (is_valid_v<operator_lshift, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_lshift, [](T& lhs, std::istream& rhs) -> decltype(auto) { return rhs >> lhs; });
			if constexpr (is_valid_v<operator_rshift, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_rshift, [](const T& lhs, std::ostream& rhs) -> decltype(auto) { return rhs << lhs; });
			if constexpr (is_valid_v<operator_lshift, T, std::size_t>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_lshift, [](const T& lhs, std::size_t rhs) -> decltype(auto) { return lhs << rhs; });
			if constexpr (is_valid_v<operator_rshift, T, std::size_t>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_rshift, [](const T& lhs, std::size_t rhs) -> decltype(auto) { return lhs >> rhs; });

			if constexpr (is_valid_v<operator_pre_inc, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_pre_inc, [](T& lhs) -> decltype(auto) { return ++lhs; });
			if constexpr (is_valid_v<operator_post_inc, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_post_inc, [](T& lhs, int) -> decltype(auto) { return lhs++; });
			if constexpr (is_valid_v<operator_pre_dec, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_pre_dec, [](T& lhs) -> decltype(auto) { return --lhs; });
			if constexpr (is_valid_v<operator_post_dec, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_post_dec, [](T& lhs, int) -> decltype(auto) { return lhs--; });

			if constexpr (is_valid_v<operator_assign_copy, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign, [](T& lhs, const T& rhs) { return lhs = rhs; });
			if constexpr (is_valid_v<operator_assign_move, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign, [](T& lhs, T&& rhs) { return lhs = std::move(rhs); });
			if constexpr (is_valid_v<operator_assign_add, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_add, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs += rhs; });
			if constexpr (is_valid_v<operator_assign_sub, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_sub, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs -= rhs; });
			if constexpr (is_valid_v<operator_assign_mul, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_mul, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs *= rhs; });
			if constexpr (is_valid_v<operator_assign_div, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_div, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs /= rhs; });
			if constexpr (is_valid_v<operator_assign_mod, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_mod, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs %= rhs; });
			if constexpr (is_valid_v<operator_assign_band, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_band, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs &= rhs; });
			if constexpr (is_valid_v<operator_assign_bor, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_bor, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs |= rhs; });
			if constexpr (is_valid_v<operator_assign_bxor, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_bxor, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs ^= rhs; });
			if constexpr (is_valid_v<operator_assign_lshift, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_lshift, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs <<= rhs; });
			if constexpr (is_valid_v<operator_assign_rshift, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_rshift, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs >>= rhs; });
			if constexpr (is_valid_v<operator_assign_lshift, T, std::size_t>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_lshift, [](T& lhs, std::size_t rhs) -> decltype(auto) { return lhs <<= rhs; });
			if constexpr (is_valid_v<operator_assign_rshift, T, std::size_t>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_assign_rshift, [](T& lhs, std::size_t rhs) -> decltype(auto) { return lhs >>= rhs; });

			if constexpr (is_valid_v<operator_eq, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_eq, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs == rhs; });
			if constexpr (is_valid_v<operator_ne, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_ne, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs != rhs; });
			if constexpr (is_valid_v<operator_lt, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_lt, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs < rhs; });
			if constexpr (is_valid_v<operator_gt, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_gt, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs > rhs; });
			if constexpr (is_valid_v<operator_le, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_le, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs <= rhs; });
			if constexpr (is_valid_v<operator_ge, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_ge, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs >= rhs; });


			if constexpr (is_valid_v<operator_subscript, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_subscript, [](T& lhs, std::size_t rhs) -> decltype(auto) { return lhs[rhs]; });
			if constexpr (is_valid_v<operator_subscript_const, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_subscript, [](const T& lhs, std::size_t rhs) -> decltype(auto) { return lhs[rhs]; });
			if constexpr (is_valid_v<operator_deref, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_deref, [](T& lhs) -> decltype(auto) { return *lhs; });
			if constexpr (is_valid_v<operator_deref_const, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_deref, [](const T& lhs) -> decltype(auto) { return *lhs; });
			/*if constexpr (is_valid_v<operator_ref, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_ref, [](T& lhs) { return &lhs; });
			if constexpr (is_valid_v<operator_ref_const, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_ref, [](const T& lhs) { return &lhs; });*/
			/*if constexpr (is_valid_v<operator_member, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_member, [](T& lhs) { return lhs.operator->(); });
			if constexpr (is_valid_v<operator_member_const, T>)
				mngr.AddMemberMethod(StrIDRegistry::Meta::operator_member, [](const T& lhs) { return lhs.operator->(); });*/

			// iterator

			if constexpr (is_iterator_v<T>) {
				if constexpr (is_valid_v<operator_add, T, std::size_t>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::operator_add, [](const T& lhs, std::size_t rhs) { return lhs + rhs; });
				if constexpr (is_valid_v<operator_sub, T, std::size_t>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::operator_sub, [](const T& lhs, std::size_t rhs) { return lhs - rhs; });
			}

			// container

			if constexpr (IsContainer_v<T>) {

				// - iterator

				if constexpr (is_valid_v<container_begin, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_begin, [](T& lhs) -> decltype(auto) { return lhs.begin(); });
				if constexpr (is_valid_v<container_begin_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_begin, [](const T& lhs) -> decltype(auto) { return lhs.begin(); });
				if constexpr (is_valid_v<container_cbegin, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_begin, [](const T& lhs) -> decltype(auto) { return lhs.cbegin(); });
				if constexpr (is_valid_v<container_end, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_end, [](T& lhs) -> decltype(auto) { return lhs.end(); });
				if constexpr (is_valid_v<container_end_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_end, [](const T& lhs) -> decltype(auto) { return lhs.end(); });
				if constexpr (is_valid_v<container_cend, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_end, [](const T& lhs) -> decltype(auto) { return lhs.cend(); });

				if constexpr (is_valid_v<container_rbegin, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_rbegin, [](T& lhs) -> decltype(auto) { return lhs.rbegin(); });
				if constexpr (is_valid_v<container_rbegin_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_rbegin, [](const T& lhs) -> decltype(auto) { return lhs.rbegin(); });
				if constexpr (is_valid_v<container_crbegin, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_crbegin, [](const T& lhs) -> decltype(auto) { return lhs.crbegin(); });
				if constexpr (is_valid_v<container_rend, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_rend, [](T& lhs) -> decltype(auto) { return lhs.rend(); });
				if constexpr (is_valid_v<container_rend_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_rend, [](const T& lhs) -> decltype(auto) { return lhs.rend(); });
				if constexpr (is_valid_v<container_crend, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_crend, [](const T& lhs) -> decltype(auto) { return lhs.crend(); });

				// - element access

				if constexpr (is_valid_v<container_at, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_at, [](T& lhs, std::size_t n) -> decltype(auto) { return lhs.at(n); });
				if constexpr (is_valid_v<container_at_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_at, [](const T& lhs, std::size_t n) -> decltype(auto) { return lhs.at(n); });

				if constexpr (is_valid_v<container_at_key, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_at, [](T& lhs, const typename T::key_type& key) -> decltype(auto) { return lhs.at(key); });
				if constexpr (is_valid_v<container_at_key_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_at, [](const T& lhs, const typename T::key_type& key) -> decltype(auto) { return lhs.at(key); });

				if constexpr (is_valid_v<container_subscript_key_0, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::operator_subscript, [](T& lhs, const typename T::node_type& key) -> decltype(auto) { return lhs[key]; });
				if constexpr (is_valid_v<container_subscript_key_1, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::operator_subscript, [](T& lhs, typename T::node_type&& key) -> decltype(auto) { return lhs[std::move(key)]; });

				if constexpr (is_valid_v<container_data, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_data, [](T& lhs) -> decltype(auto) { return lhs.data(); });
				if constexpr (is_valid_v<container_data_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_data, [](const T& lhs) -> decltype(auto) { return lhs.data(); });

				if constexpr (is_valid_v<container_front, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_front, [](T& lhs) -> decltype(auto) { return lhs.front(); });
				if constexpr (is_valid_v<container_front_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_front, [](const T& lhs) -> decltype(auto) { return lhs.front(); });

				if constexpr (is_valid_v<container_back, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_back, [](T& lhs) -> decltype(auto) { return lhs.back(); });
				if constexpr (is_valid_v<container_back_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_back, [](const T& lhs) -> decltype(auto) { return lhs.back(); });

				if constexpr (is_valid_v<container_empty, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_empty, [](const T& lhs) -> decltype(auto) { return lhs.empty(); });

				if constexpr (is_valid_v<container_size, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_size, [](const T& lhs) -> decltype(auto) { return lhs.size(); });

				/*if constexpr (is_valid_v<container_max_size, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_max_size, [](const T& lhs) -> decltype(auto) { return lhs.max_size(); });*/

				if constexpr (is_valid_v<container_resize_0, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_resize, [](T& lhs, std::size_t n) -> decltype(auto) { return lhs.resize(n); });

				if constexpr (is_valid_v<container_resize_1, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_resize, [](T& lhs, std::size_t n, const typename T::value_type& value) -> decltype(auto) { return lhs.resize(n, value); });

				if constexpr (is_valid_v<container_capacity, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_capacity, [](const T& lhs) -> decltype(auto) { return lhs.capacity(); });

				if constexpr (is_valid_v<container_bucket_count, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_bucket_count, [](const T& lhs) -> decltype(auto) { return lhs.bucket_count(); });

				if constexpr (is_valid_v<container_reserve, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_reserve, [](T& lhs, std::size_t n) -> decltype(auto) { return lhs.reserve(n); });

				if constexpr (is_valid_v<container_shrink_to_fit, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_shrink_to_fit, [](T& lhs) -> decltype(auto) { return lhs.shrink_to_fit(); });

				// - modifiers

				if constexpr (is_valid_v<container_clear, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_clear, [](T& lhs) -> decltype(auto) { return lhs.clear(); });

				if constexpr (is_valid_v<container_insert_0, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator iter, const typename T::value_type& value) -> decltype(auto) { return lhs.insert(iter, value); });

				if constexpr (is_valid_v<container_insert_1, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator iter, typename T::value_type&& value) -> decltype(auto) { return lhs.insert(iter, std::move(value)); });

				if constexpr (is_valid_v<container_insert_2, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator iter, std::size_t n, const typename T::value_type& value) -> decltype(auto) { return lhs.insert(iter, n, std::move(value)); });

				if constexpr (is_valid_v<container_insert_3, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_insert, [](T& lhs, typename T::node_type&& value) -> decltype(auto) { return lhs.insert(std::move(value)); });

				if constexpr (is_valid_v<container_insert_4, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator iter, typename T::node_type&& value) -> decltype(auto) { return lhs.insert(iter, std::move(value)); });

				// assign (TODO)

				if constexpr (is_valid_v<container_erase_0, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_insert, [](T& lhs, typename T::iterator iter) -> decltype(auto) { return lhs.erase(iter); });

				if constexpr (is_valid_v<container_erase_1, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator iter) -> decltype(auto) { return lhs.erase(iter); });

				if constexpr (is_valid_v<container_erase_2, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_insert, [](T& lhs, typename T::iterator start, typename T::iterator end) -> decltype(auto) { return lhs.erase(start, end); });

				if constexpr (is_valid_v<container_erase_3, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_insert, [](T& lhs, typename T::const_iterator start, typename T::const_iterator end) -> decltype(auto) { return lhs.erase(start, end); });

				if constexpr (is_valid_v<container_erase_4, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_insert, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.erase(rhs); });

				if constexpr (is_valid_v<container_push_front_0, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_push_front, [](T& lhs, const typename T::value_type& value) -> decltype(auto) { return lhs.push_front(value); });

				if constexpr (is_valid_v<container_push_front_1, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_push_front, [](T& lhs, typename T::value_type&& value) -> decltype(auto) { return lhs.push_front(std::move(value)); });

				if constexpr (is_valid_v<container_pop_front, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_pop_front, [](T& lhs) -> decltype(auto) { return lhs.pop_front(); });

				if constexpr (is_valid_v<container_push_back_0, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_push_back, [](T& lhs, const typename T::value_type& value) -> decltype(auto) { return lhs.push_back(value); });
				if constexpr (is_valid_v<container_push_back_1, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_push_back, [](T& lhs, typename T::value_type&& value) -> decltype(auto) { return lhs.push_back(std::move(value)); });

				if constexpr (is_valid_v<container_pop_back, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_pop_back, [](T& lhs) -> decltype(auto) { return lhs.pop_back(); });

				if constexpr (is_valid_v<container_swap, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_swap, [](T& lhs, T& rhs) -> decltype(auto) { return std::swap(lhs, rhs); });

				if constexpr (is_valid_v<container_extract_0, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_extract, [](T& lhs, typename T::const_iterator iter) -> decltype(auto) { return lhs.extract(iter); });

				if constexpr (is_valid_v<container_extract_1, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_extract, [](T& lhs, const typename T::key_type& key) -> decltype(auto) { return lhs.extract(key); });

				if constexpr (is_valid_v<container_merge_0, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_merge, [](T& lhs, const T& rhs) -> decltype(auto) { return lhs.merge(rhs); });
				if constexpr (is_valid_v<container_merge_1, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_merge, [](T& lhs, T&& rhs) -> decltype(auto) { return lhs.merge(std::move(rhs)); });

				// - lookup

				if constexpr (is_valid_v<container_count, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_count, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.count(rhs); });

				if constexpr (is_valid_v<container_find, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_find, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.find(rhs); });
				if constexpr (is_valid_v<container_find_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_find, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.find(rhs); });

				if constexpr (is_valid_v<container_lower_bound, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_lower_bound, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.lower_bound(rhs); });
				if constexpr (is_valid_v<container_lower_bound_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_lower_bound, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.lower_bound(rhs); });

				if constexpr (is_valid_v<container_upper_bound, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_upper_bound, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.upper_bound(rhs); });
				if constexpr (is_valid_v<container_upper_bound_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_upper_bound, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.upper_bound(rhs); });

				if constexpr (is_valid_v<container_equal_range, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_equal_range, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.equal_range(rhs); });
				if constexpr (is_valid_v<container_equal_range_const, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_equal_range, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.equal_range(rhs); });

				// - observers

				if constexpr (is_valid_v<container_key_comp, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_key_comp, [](const T& lhs) -> decltype(auto) { return lhs.key_comp(); });

				if constexpr (is_valid_v<container_value_comp, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_value_comp, [](const T& lhs) -> decltype(auto) { return lhs.value_comp(); });

				if constexpr (is_valid_v<container_hash_function, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_hash_function, [](const T& lhs) -> decltype(auto) { return lhs.hash_function(); });

				if constexpr (is_valid_v<container_key_eq, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_key_eq, [](const T& lhs) -> decltype(auto) { return lhs.key_eq(); });

				if constexpr (is_valid_v<container_get_allocator, T>)
					mngr.AddMemberMethod(StrIDRegistry::Meta::container_get_allocator, [](const T& lhs) -> decltype(auto) { return lhs.get_allocator(); });

				if constexpr (IsVector_v<T>)
					mngr.AddAttr(TypeID::of<T>, mngr.MakeShared(TypeID::of<ContainerType>, ContainerType::VECTOR));
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
			static_assert(!std::is_void_v<Value> && !std::is_function_v<Value> && !std::is_volatile_v<Value>);
			using ConstFlag = std::bool_constant<std::is_const_v<Value>>;
			tregistry.Register<Value>();
			RegisterTypeAuto<std::remove_const_t<Value>>();
			return {
				TypeID::of<std::remove_const_t<Value>>,
				field_data
			};
		}
		else if constexpr (std::is_member_object_pointer_v<FieldData>) {
			using Traits = member_pointer_traits<FieldData>;
			using Object = typename Traits::object;
			using Value = typename Traits::value;
			using ConstFlag = std::bool_constant<std::is_const_v<Value>>;
			static_assert(!std::is_function_v<Value> && !std::is_volatile_v<Value>);

			tregistry.Register<Value>();
			RegisterTypeAuto<std::remove_const_t<Value>>();
			if constexpr (has_virtual_base_v<Object>) {
				return {
					TypeID::of<std::remove_const_t<Value>>,
					field_offsetor<field_data>(),
					ConstFlag{}
				};
			}
			else {
				return {
					TypeID::of<std::remove_const_t<Value>>,
					field_forward_offset_value(field_data),
					ConstFlag{}
				};
			}
		}
		else if constexpr (std::is_enum_v<FieldData>) {
			using Value = std::remove_pointer_t<FieldData>;
			tregistry.Register<Value>();
			RegisterTypeAuto<std::remove_const_t<Value>>();
			const auto buffer = FieldPtr::ConvertToBuffer(field_data);
			return {
				TypeID::of<std::remove_const_t<Value>>,
				buffer
			};
		}
		else
			static_assert(always_false<FieldData>);
	}

	template<typename T>
	FieldPtr ReflMngr::GenerateFieldPtr(T&& data) {
		using RawT = std::decay_t<T>;
		static_assert(!std::is_same_v<RawT, size_t>);
		if constexpr (std::is_member_object_pointer_v<RawT>) {
			using Traits = member_pointer_traits<RawT>;
			using Object = typename Traits::object;
			using Value = typename Traits::value;
			using ConstFlag = std::bool_constant<std::is_const_v<Value>>;
			static_assert(!std::is_reference_v<Value> && !std::is_volatile_v<Value>);
			tregistry.Register<Value>();
			RegisterTypeAuto<std::remove_const_t<Value>>();
			if constexpr (has_virtual_base_v<Object>) {
				return {
					TypeID::of<std::remove_const_t<Value>>,
					field_offsetor(data),
					ConstFlag{}
				};
			}
			else {
				return {
					TypeID::of<std::remove_const_t<Value>>,
					field_forward_offset_value(data),
					ConstFlag{}
				};
			}
		}
		else if constexpr (std::is_pointer_v<RawT> && !is_function_pointer_v<RawT> && std::is_void_v<std::remove_pointer_t<RawT>>) {
			using Value = std::remove_pointer_t<RawT>;
			static_assert(!std::is_volatile_v<Value>);
			tregistry.Register<Value>();
			RegisterTypeAuto<std::remove_const_t<Value>>();
			return {
				TypeID::of<std::remove_const_t<Value>>,
				data,
				std::bool_constant<std::is_const_v<Value>>{}
			};
		}
		else if constexpr (std::is_enum_v<RawT>) {
			tregistry.Register<RawT>();
			RegisterTypeAuto<std::remove_const_t<RawT>>();
			const auto buffer = FieldPtr::ConvertToBuffer(data);
			return {
				TypeID::of<std::remove_const_t<RawT>>,
				buffer
			};
		}
		else {
			using Traits = FuncTraits<RawT>;

			using ArgList = typename Traits::ArgList;
			static_assert(Length_v<ArgList> == 1);
			using ObjectPtr = Front_t<ArgList>;
			static_assert(std::is_pointer_v<ObjectPtr>);
			using Obj = std::remove_pointer_t<ObjectPtr>;
			static_assert(!std::is_const_v<Obj>);

			using ValuePtr = typename Traits::Return;
			static_assert(std::is_pointer_v<ValuePtr>);
			using Value = std::remove_pointer_t<ValuePtr>;
			static_assert(!std::is_void_v<Value> && !std::is_volatile_v<Value>);

			tregistry.Register<Value>();
			RegisterTypeAuto<std::remove_const_t<Value>>();
			using ConstFlag = std::bool_constant<std::is_const_v<Value>>;

			auto offsetor = [f=std::forward<T>(data)](const void* obj) -> const void* {
				return f(const_cast<Obj*>(reinterpret_cast<const Obj*>(obj)));
			};

			return {
				TypeID::of<std::remove_const_t<Value>>,
				offsetor,
				ConstFlag{}
			};
		}
	}

	template<typename T, typename... Args>
	FieldPtr ReflMngr::GenerateDynamicFieldPtr(Args&&... args) {
		static_assert(!std::is_reference_v<T> && !std::is_volatile_v<T>);
		using RawT = std::remove_const_t<T>;
		if constexpr (FieldPtr::IsBufferable<RawT>()) {
			FieldPtr::Buffer buffer = FieldPtr::ConvertToBuffer(T{ std::forward<Args>(args)... });
			return FieldPtr{ TypeID::of<RawT>, buffer, std::bool_constant<std::is_const_v<T>>{} };
		}
		else {
			static_assert(alignof(RawT) <= alignof(max_align_t));
			using MaybeConstSharedObject = std::conditional_t<std::is_const_v<T>, SharedConstObject, SharedObject>;
			MaybeConstSharedObject obj{ TypeID::of<RawT>, std::make_shared<RawT>(std::forward<Args>(args)...) };
			return FieldPtr{ obj };
		}
	}

	template<typename T, typename Alloc, typename... Args>
	FieldPtr ReflMngr::GenerateDynamicFieldPtrByAlloc(const Alloc& alloc, Args&&... args) {
		static_assert(!std::is_reference_v<T> && !std::is_volatile_v<T>);
		using RawT = std::remove_const_t<T>;
		if constexpr (FieldPtr::IsBufferable<RawT>()) {
			FieldPtr::Buffer buffer = FieldPtr::ConvertToBuffer(T{ std::forward<Args>(args)... });
			return FieldPtr{ TypeID::of<RawT>, buffer, std::bool_constant<std::is_const_v<T>>{} };
		}
		else {
			using MaybeConstSharedObject = std::conditional_t<std::is_const_v<T>, SharedConstObject, SharedObject>;
			MaybeConstSharedObject obj = { TypeID::of<RawT>, std::allocate_shared<RawT>(alloc, std::forward<Args>(args)...) };
			return FieldPtr{ obj };
		}
	}

	template<typename Return>
	ResultDesc ReflMngr::GenerateResultDesc() {
		if constexpr (!std::is_void_v<Return>) {
			static_assert(!std::is_const_v<Return> && !std::is_volatile_v<Return> && !std::is_volatile_v<std::remove_reference_t<Return>>);
			using U = std::conditional_t<std::is_reference_v<Return>, std::add_pointer_t<Return>, Return>;
			tregistry.Register<Return>();
			RegisterTypeAuto<Return>();
			return {
				TypeID::of<Return>,
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
			static_assert(((!std::is_const_v<Params> && !std::is_volatile_v<Params> && !std::is_volatile_v<std::remove_reference_t<Params>>)&&...));
			(tregistry.Register<Params>(), ...);
			(RegisterTypeAuto<Params>(), ...);
			return { { TypeID::of<Params>... } };
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
			Helper::GenerateParamList(*this)
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
			Helper::GenerateParamList(*this)
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
			Helper::GenerateParamList(*this)
		};
	}

	//
	// Modifier
	/////////////

	template<typename T>
	void ReflMngr::RegisterType() {
		RegisterTypeAuto<T>();
	}

	template<typename T>
	void ReflMngr::RegisterTypeAuto(AttrSet attrs_ctor, AttrSet attrs_dtor) {
		static_assert(!std::is_volatile_v<T>);
		if constexpr (std::is_void_v<T>)
			return;
		else {
			if constexpr (std::is_const_v<T>)
				RegisterTypeAuto<std::remove_const_t<T>>(std::move(attrs_ctor), std::move(attrs_dtor));
			else if constexpr (std::is_reference_v<T>)
				RegisterTypeAuto<std::remove_cv_t<std::remove_reference_t<T>>>(std::move(attrs_ctor), std::move(attrs_dtor));
			else if constexpr (std::is_pointer_v<T>)
				RegisterTypeAuto<std::remove_cv_t<std::remove_pointer_t<T>>>(std::move(attrs_ctor), std::move(attrs_dtor));
			else {
				if (IsRegistered(TypeID::of<T>))
					return;
				RegisterType(type_name<T>(), sizeof(T), alignof(T));

				if constexpr (std::is_default_constructible_v<T>)
					AddConstructor<T>(std::move(attrs_ctor));
				if constexpr (std::is_copy_constructible_v<T>)
					AddConstructor<T, const T&>(std::move(attrs_ctor));
				if constexpr (std::is_move_constructible_v<T>)
					AddConstructor<T, T&&>(std::move(attrs_ctor));
				if constexpr (std::is_destructible_v<T>)
					AddDestructor<T>(std::move(attrs_dtor));

				details::TypeAutoRegister<T>::run(*this);
			}
		}
	}

	template<auto field_data>
	StrID ReflMngr::AddField(std::string_view name, AttrSet attrs) {
		using FieldData = decltype(field_data);
		if constexpr (std::is_enum_v<FieldData>) {
			return AddField(
				TypeID::of<std::remove_const_t<FieldData>>,
				name,
				{ GenerateFieldPtr<field_data>(), std::move(attrs) }
			);
		}
		else if constexpr (std::is_pointer_v<FieldData>) {
			return AddField(
				TypeID::of<std::remove_const_t<std::remove_pointer_t<FieldData>>>,
				name,
				{ GenerateFieldPtr<field_data>(), std::move(attrs) }
			);
		}
		else {
			return AddField(
				TypeID::of<std::remove_const_t<member_pointer_traits_object<FieldData>>>,
				name,
				{ GenerateFieldPtr<field_data>(), std::move(attrs) }
			);
		}
	}


	template<typename T,
		std::enable_if_t<!std::is_same_v<std::decay_t<T>, FieldInfo>, int>>
	StrID ReflMngr::AddField(std::string_view name, T&& data, AttrSet attrs) {
		using RawT = std::remove_cv_t<std::remove_reference_t<T>>;
		if constexpr (std::is_member_object_pointer_v<RawT>)
			return AddField(TypeID::of<member_pointer_traits_object<RawT>>, name, std::forward<T>(data), std::move(attrs));
		else if constexpr (std::is_enum_v<RawT>)
			return AddField(TypeID::of<RawT>, name, std::forward<T>(data), std::move(attrs));
		else {
			using Traits = FuncTraits<RawT>;

			using ArgList = typename Traits::ArgList;
			static_assert(Length_v<ArgList> == 1);
			using ObjectPtr = Front_t<ArgList>;
			static_assert(std::is_pointer_v<ObjectPtr>);
			using Obj = std::remove_pointer_t<ObjectPtr>;
			static_assert(!std::is_const_v<Obj> && !std::is_volatile_v<Obj>);

			return AddField(TypeID::of<Obj>, name, std::forward<T>(data), std::move(attrs));
		}
	}

	template<auto member_func_ptr>
	StrID ReflMngr::AddMethod(std::string_view name, AttrSet attrs) {
		using MemberFuncPtr = decltype(member_func_ptr);
		static_assert(std::is_member_function_pointer_v<MemberFuncPtr>);
		using Obj = member_pointer_traits_object<MemberFuncPtr>;
		return AddMethod(
			TypeID::of<Obj>,
			name,
			{ GenerateMethodPtr<member_func_ptr>(), std::move(attrs) }
		);
	}

	template<auto func_ptr>
	StrID ReflMngr::AddMethod(TypeID typeID, std::string_view name, AttrSet attrs) {
		using FuncPtr = decltype(func_ptr);
		static_assert(std::is_function_v<FuncPtr>);
		return AddMethod(
			typeID,
			name,
			{ GenerateMethodPtr<func_ptr>(), std::move(attrs) }
		);
	}

	template<typename T, typename... Args>
	bool ReflMngr::AddConstructor(AttrSet attrs) {
		return AddMethod(
			TypeID::of<T>,
			StrIDRegistry::Meta::ctor,
			{ GenerateConstructorPtr<T, Args...>() , std::move(attrs) }
		).Valid();
	}
	template<typename T>
	bool ReflMngr::AddDestructor(AttrSet attrs) {
		return AddMethod(
			TypeID::of<T>,
			StrIDRegistry::Meta::dtor,
			{ GenerateDestructorPtr<T>() , std::move(attrs) }
		).Valid();
	}

	template<typename Func>
	StrID ReflMngr::AddMemberMethod(std::string_view name, Func&& func, AttrSet attrs) {
		return AddMethod(
			TypeID::of<typename details::WrapFuncTraits<std::decay_t<Func>>::Object>,
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
		return (AddBase(TypeID::of<Derived>, TypeID::of<Bases>, GenerateBaseInfo<Derived, Bases>()) && ...);
	}

	//
	// Invoke
	///////////

	template<typename... Args>
	InvocableResult ReflMngr::IsStaticInvocable(TypeID typeID, StrID methodID) const noexcept {
		std::array argTypeIDs = { TypeID::of<Args>... };
		return IsStaticInvocable(typeID, methodID, Span<const TypeID>{argTypeIDs});
	}

	template<typename... Args>
	InvocableResult ReflMngr::IsConstInvocable(TypeID typeID, StrID methodID) const noexcept {
		std::array argTypeIDs = { TypeID::of<Args>... };
		return IsConstInvocable(typeID, methodID, Span<const TypeID>{argTypeIDs});
	}

	template<typename... Args>
	InvocableResult ReflMngr::IsInvocable(TypeID typeID, StrID methodID) const noexcept {
		std::array argTypeIDs = { TypeID::of<Args>... };
		return IsInvocable(typeID, methodID, Span<const TypeID>{argTypeIDs});
	}

	template<typename T>
	T ReflMngr::InvokeRet(TypeID typeID, StrID methodID, Span<const TypeID> argTypeIDs, void* args_buffer) const {
		using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
		std::uint8_t result_buffer[sizeof(U)];
		InvokeResult result = Invoke(typeID, methodID, result_buffer, argTypeIDs, args_buffer);
		assert(result.resultID == TypeID::of<T>);
		return result.Move<T>(result_buffer);
	}

	template<typename T>
	T ReflMngr::InvokeRet(ConstObjectPtr obj, StrID methodID, Span<const TypeID> argTypeIDs, void* args_buffer) const {
		using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
		std::uint8_t result_buffer[sizeof(U)];
		InvokeResult result = Invoke(obj, methodID, result_buffer, argTypeIDs, args_buffer);
		assert(result.resultID == TypeID::of<T>);
		return result.Move<T>(result_buffer);
	}

	template<typename T>
	T ReflMngr::InvokeRet(ObjectPtr obj, StrID methodID, Span<const TypeID> argTypeIDs, void* args_buffer) const {
		using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
		std::uint8_t result_buffer[sizeof(U)];
		InvokeResult result = Invoke(obj, methodID, result_buffer, argTypeIDs, args_buffer);
		assert(result.resultID == TypeID::of<T>);
		return result.Move<T>(result_buffer);
	}

	template<typename... Args>
	InvokeResult ReflMngr::InvokeArgs(TypeID typeID, StrID methodID, void* result_buffer, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return Invoke(typeID, methodID, result_buffer, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return Invoke(typeID, methodID, result_buffer);
	}

	template<typename... Args>
	InvokeResult ReflMngr::InvokeArgs(ConstObjectPtr obj, StrID methodID, void* result_buffer, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return Invoke(obj, methodID, result_buffer, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return Invoke(obj, methodID, result_buffer);
	}

	template<typename... Args>
	InvokeResult ReflMngr::InvokeArgs(ObjectPtr obj, StrID methodID, void* result_buffer, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return Invoke(obj, methodID, result_buffer, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return Invoke(obj, methodID, result_buffer);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(TypeID typeID, StrID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return InvokeRet<T>(typeID, methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return InvokeRet<T>(typeID, methodID);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(ConstObjectPtr obj, StrID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return InvokeRet<T>(obj, methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return InvokeRet<T>(obj, methodID);
	}

	template<typename T, typename... Args>
	T ReflMngr::Invoke(ObjectPtr obj, StrID methodID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return InvokeRet<T>(obj, methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return InvokeRet<T>(obj, methodID);
	}

	//
	// Meta
	/////////

	template<typename... Args>
	bool ReflMngr::IsConstructible(TypeID typeID) const noexcept {
		std::array argTypeIDs = { TypeID::of<Args>... };
		return IsConstructible(typeID, Span<const TypeID>{argTypeIDs});
	}

	template<typename... Args>
	bool ReflMngr::Construct(ObjectPtr obj, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return Construct(obj, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return Construct(obj);
	}

	template<typename... Args>
	ObjectPtr ReflMngr::New(TypeID typeID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return New(typeID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return New(typeID);
	}

	template<typename T, typename... Args>
	ObjectPtr ReflMngr::NewAuto(Args... args) {
		static_assert(!std::is_const_v<T> && !std::is_volatile_v<T> && !std::is_reference_v<T>);
		RegisterTypeAuto<T, Args...>();
		AddMethod(TypeID::of<T>, StrIDRegistry::Meta::ctor, { GenerateConstructorPtr<T, Args...>() });
		return New(TypeID::of<T>, std::forward<Args>(args)...);
	}

	template<typename... Args>
	SharedObject ReflMngr::MakeShared(TypeID typeID, Args... args) const {
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return MakeShared(typeID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()));
		}
		else
			return MakeShared(typeID);
	}

	template<typename T, typename... Args>
	SharedObject ReflMngr::MakeSharedAuto(Args... args) {
		static_assert(!std::is_const_v<T> && !std::is_volatile_v<T> && !std::is_reference_v<T>);
		RegisterTypeAuto<T, Args...>();
		AddMethod(TypeID::of<T>, StrIDRegistry::Meta::ctor, { GenerateConstructorPtr<T, Args...>() });
		return MakeShared(TypeID::of<T>, std::forward<Args>(args)...);
	}

	//
	// Memory
	///////////

	template<typename... Args>
	SharedObject ReflMngr::MInvoke(
		TypeID typeID,
		StrID methodID,
		std::pmr::memory_resource* rst_rsrc,
		Args... args)
	{
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return MInvoke(typeID, methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()), rst_rsrc);
		}
		else
			return MInvoke(typeID, methodID, Span<const TypeID>{}, static_cast<void*>(nullptr), rst_rsrc);
	}

	template<typename... Args>
	SharedObject ReflMngr::MInvoke(
		ConstObjectPtr obj,
		StrID methodID,
		std::pmr::memory_resource* rst_rsrc,
		Args... args)
	{
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return MInvoke(obj, methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()), rst_rsrc);
		}
		else
			return MInvoke(obj, methodID, Span<const TypeID>{}, static_cast<void*>(nullptr), rst_rsrc);
	}

	template<typename... Args>
	SharedObject ReflMngr::MInvoke(
		ObjectPtr obj,
		StrID methodID,
		std::pmr::memory_resource* rst_rsrc,
		Args... args)
	{
		if constexpr (sizeof...(Args) > 0) {
			static_assert(!((std::is_const_v<Args> || std::is_volatile_v<Args>) || ...));
			std::array argTypeIDs = { TypeID::of<Args>... };
			std::array args_buffer{ reinterpret_cast<std::size_t>(&args)... };
			return MInvoke(obj, methodID, Span<const TypeID>{ argTypeIDs }, static_cast<void*>(args_buffer.data()), rst_rsrc);
		}
		else
			return MInvoke(obj, methodID, Span<const TypeID>{}, static_cast<void*>(nullptr), rst_rsrc);
	}

	template<typename... Args>
	SharedObject ReflMngr::DMInvoke(
		TypeID typeID,
		StrID methodID,
		Args... args)
	{
		return MInvoke<Args...>(typeID, methodID, std::pmr::get_default_resource(), std::forward<Args>(args)...);
	}

	template<typename... Args>
	SharedObject ReflMngr::DMInvoke(
		ConstObjectPtr obj,
		StrID methodID,
		Args... args)
	{
		return MInvoke<Args...>(obj, methodID, std::pmr::get_default_resource(), std::forward<Args>(args)...);
	}

	template<typename... Args>
	SharedObject ReflMngr::DMInvoke(
		ObjectPtr obj,
		StrID methodID,
		Args... args)
	{
		return MInvoke<Args...>(obj, methodID, std::pmr::get_default_resource(), std::forward<Args>(args)...);
	}
}
