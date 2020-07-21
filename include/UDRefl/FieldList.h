#pragma once

#include "Field.h"

#include <map>

namespace Ubpa::UDRefl {
	struct FieldList {
		static constexpr const char default_constructor[] = "__default_constructor";
		static constexpr const char copy_constructor[] = "__copy_constructor";
		static constexpr const char move_constructor[] = "__move_constructor";
		static constexpr const char destructor[] = "__destructor";
		static constexpr const char enum_value[] = "__enum_value";

		std::multimap<std::string, Field, std::less<>> data;
		using Iterator = std::multimap<std::string, Field, std::less<>>::iterator;
		using ConstIterator = std::multimap<std::string, Field, std::less<>>::const_iterator;

		// static
		template<typename T>
		T& Get(std::string_view name) {
			static_assert(!std::is_reference_v<T>);
			assert(data.count(name) == 1);
			StaticVar& v = data.find(name)->second.value.Cast<StaticVar>();
			return v.Cast<T>();
		}

		// static
		template<typename T>
		const T& Get(std::string_view name) const {
			return const_cast<FieldList*>(this)->Get<T>(name);
		}

		template<typename T>
		T& Get(std::string_view name, Object obj) const {
			assert(data.count(name) == 1);
			auto& v = data.find(name)->second.value.Cast<Var>();
			return v.Get<T>(obj);
		}

		template<typename Arg>
		void Set(std::string_view name, Object obj, Arg arg) const {
			Get<Arg>(name, obj) = std::forward<Arg>(arg);
		}

		// static
		template<typename T>
		std::pair<std::string_view, Field*> FindStaticField(const T& value) {
			for (auto iter = data.begin(); iter != data.end(); ++iter) {
				if (auto pV = iter->second.value.CastIf<StaticVar>()) {
					if ((*pV) == value)
						return { iter->first, &iter->second };
				}
			}
			return { "", nullptr };
		}

		AnyWrapper Call(std::string_view name, ArgList args) {
			auto low = data.lower_bound(name);
			auto up = data.upper_bound(name);
			for (auto iter = low; iter != up; ++iter) {
				if (auto pFunc = low->second.value.CastIf<Func>()) {
					if (pFunc->signature == args.GetFuncSig())
						return pFunc->Call(std::move(args));
				}
			}

			assert("arguments' types are matching failure with functions" && false);
			return {};
		}

		template<typename Ret, typename... Args>
		Ret Call(std::string_view name, Args... args) {
			static_assert(std::is_void_v<Ret> || std::is_constructible_v<Ret>);

			auto rst = Call(name, ArgList{ std::forward<Args>(args)... });

			if constexpr (!std::is_void_v<Ret>)
				return rst.Cast<Ret>();
		}

		void DefaultConstruct(Object obj) {
			return Call<void, Object>(default_constructor, obj);
		}

		void CopyConstruct(Object dst, Object src) {
			return Call<void, Object, Object>(copy_constructor, dst, src);
		}

		void MoveConstruct(Object dst, Object src) {
			return Call<void, Object, Object>(move_constructor, dst, src);
		}

		void Destruct(Object p) {
			return Call<void, Object>(destructor, p);
		}
	};
}