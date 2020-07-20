#pragma once

#include <map>
#include <string>
#include <any>
#include <variant>
#include <functional>
#include <vector>
#include <cassert>

namespace Ubpa::UDRefl {
	struct TypeInfo;

	class Object {
	public:
		Object(size_t id, void* ptr) : id{ id }, ptr{ ptr }{}
		Object() : id{ static_cast<size_t>(-1) }, ptr{ nullptr }{}

		void* Pointer() { return ptr; }
		size_t ID() { return id; }

		// non-static
		template<typename T>
		T& Var(size_t offset) {
			return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(ptr) + offset);
		}

	private:
		size_t id;
		void* ptr;
	};

	struct AnyWrapper {
		template<typename T>
		AnyWrapper(T data) : data{ data } {}
		AnyWrapper() = default;

		std::any data;

		bool HasValue() const {
			return data.has_value();
		}

		const std::type_info& Type() const noexcept {
			return data.type();
		}

		template<typename T>
		bool TypeIs() const noexcept {
			return Type() == typeid(T);
		}

		template<typename T>
		bool operator==(const T& t) const {
			return HasValue() && TypeIs<T>() && Cast<T>() == t;
		}

		template<typename T>
		T& Cast() {
			assert(HasValue() && TypeIs<T>());
			return std::any_cast<T&>(data);
		}

		template<typename T>
		const T& Cast() const {
			return const_cast<AnyWrapper*>(this)->Cast<T>();
		}

		template<typename T>
		T* CastIf() {
			return std::any_cast<T>(&data);
		}

		template<typename T>
		const T* CastIf() const {
			return const_cast<AnyWrapper*>(this)->CastIf<T>();
		}
	};

	template<typename... Ts>
	struct VariantWrapper {
		std::variant<Ts...> data;

		bool HasValue() const noexcept {
			return data.valueless_by_exception();
		}

		size_t Index() const noexcept {
			return data.index();
		}

		template<typename T>
		bool TypeIs() const {
			return std::holds_alternative<T>(data);
		}

		template<typename T>
		T& Cast() {
			return std::get<T>(data);
		}

		template<typename T>
		const T& Cast() const {
			return const_cast<VariantWrapper*>(this)->Cast<T>();
		}

		template<typename T>
		T* CastIf() {
			return std::get_if<T>(&data);
		}

		template<typename T>
		const T* CastIf() const {
			return const_cast<VariantWrapper*>(this)->CastIf<T>();
		}
	};

	struct Attr : AnyWrapper {
		using AnyWrapper::AnyWrapper;
	};

	using AttrList = std::map<std::string, Attr, std::less<>>;

	struct Field {
		struct Var {
			std::any getter;

			template<typename T>
			static Var Init(size_t offset) {
				return {
					std::function{[=](Object obj)->T& {
						return obj.Var<T>(offset);
					}}
				};
			}

			template<typename T>
			bool TypeIs() const noexcept {
				return getter.type() == typeid(std::function<T& (Object)>);
			}

			template<typename T>
			T& Get(Object obj) const {
				assert(TypeIs<T>());
				return std::any_cast<std::function<T& (Object)>>(getter)(obj);
			}

			template<typename Arg>
			void Set(Object obj, Arg arg) const {
				Get<Arg>(obj) = std::forward<Arg>(arg);
			}
		};
		struct StaticVar : AnyWrapper {
			using AnyWrapper::AnyWrapper;
		};
		struct Func : AnyWrapper {
			template<typename T>
			Func(T func) : AnyWrapper{ std::function{func} } {}
			template<typename T>
			bool FuncTypeIs() const {
				return TypeIs<std::function<T>>();
			}
			template<typename Ret, typename... Args>
			Ret Call(Args... args) const {
				return Cast<std::function<Ret(Args...)>>()(std::forward<Args>(args)...);
			}
		};
		using Value = VariantWrapper<Var, StaticVar, Func>;

		Value value;
		AttrList attrs;

		bool operator<(const Field& rhs) const noexcept {
			if (!value.TypeIs<Func>() || !rhs.value.TypeIs<Func>())
				return false;
			return value.Cast<Func>().data.type().hash_code()
				< rhs.value.Cast<Func>().data.type().hash_code();
		}
	};

	struct FieldList {
		static constexpr const char default_constructor[] = "_default_constructor";
		static constexpr const char copy_constructor[] = "_copy_constructor";
		static constexpr const char move_constructor[] = "_move_constructor";
		static constexpr const char destructor[] = "_destructor";

		std::multimap<std::string, Field, std::less<>> data;

		// static
		template<typename T>
		T& Get(std::string_view name) {
			static_assert(!std::is_reference_v<T>);
			assert(data.count(name) == 1);
			Field::StaticVar& v = data.find(name)->second.value.Cast<Field::StaticVar>();
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
			auto& v = data.find(name)->second.value.Cast<Field::Var>();
			return v.Get<T>(obj);
		}

		template<typename Arg>
		void Set(std::string_view name, Object obj, Arg arg) const {
			Get<Arg>(name, obj) = std::forward<Arg>(arg);
		}

		template<typename Ret, typename... Args>
		Ret Call(std::string_view name, Args... args) const {
			static_assert(std::is_void_v<Ret> || std::is_constructible_v<Ret>);

			auto low = data.lower_bound(name);
			auto up = data.upper_bound(name);
			for (auto iter = low; iter != up; ++iter) {
				if (auto pFunc = low->second.value.CastIf<Field::Func>()) {
					if (pFunc->FuncTypeIs<Ret(Args...)>())
						return pFunc->Call<Ret, Args...>(std::forward<Args>(args)...);
				}
			}

			assert("arguments' types are matching failure with functions" && false);
			if constexpr (!std::is_void_v<Ret>)
				return {};
		}

		void DefaultConstruct(Object obj) const {
			return Call<void, Object>(default_constructor, obj);
		}

		void CopyConstruct(Object dst, Object src) const {
			return Call<void, Object, Object>(copy_constructor, dst, src);
		}

		void MoveConstruct(Object dst, Object src) const {
			return Call<void, Object, Object>(move_constructor, dst, src);
		}

		void Destruct(Object p) const {
			return Call<void, Object>(destructor, p);
		}
	};

	struct Base {
		TypeInfo* info;
		size_t offset;
		bool isVirtual{ false };
	};

	using BaseList = std::map<std::string, Base, std::less<>>;

	struct TypeInfo {
		TypeInfo(size_t ID) : ID{ ID } {}

		const size_t ID;

		std::string name;

		size_t size{ 0 };
		size_t alignment{ alignof(std::max_align_t) };

		BaseList bases;

		AttrList attrs;
		FieldList fields;

		// TODO: alignment
		// no construct
		Object Malloc() const {
			assert(size != 0);
			void* ptr = malloc(size);
			assert(ptr != nullptr);
			return { ID, ptr };
		}

		void Free(Object obj) const {
			free(obj.Pointer());
		}

		// call Allocate and fields.DefaultConstruct
		Object New() const {
			Object obj = Malloc();
			fields.DefaultConstruct(obj);
			return obj;
		}

		template<typename... Args>
		Object New(std::string_view name, Args... args) const {
			Object obj = Malloc();
			fields.Call<void, Object, Args...>(name, obj, std::forward<Args>(args)...);
			return obj;
		}

		// call Allocate and fields.DefaultConstruct
		void Delete(Object obj) const {
			if (obj.Pointer() != nullptr)
				fields.Destruct(obj);
			Free(obj);
		}

		TypeInfo(const TypeInfo&) = delete;
		TypeInfo(TypeInfo&&) = delete;
		TypeInfo& operator==(const TypeInfo&) = delete;
		TypeInfo& operator==(TypeInfo&&) = delete;
	};

	class TypeInfoMngr {
	public:
		static TypeInfoMngr& Instance() {
			static TypeInfoMngr instance;
			return instance;
		}

		TypeInfo& GetTypeInfo(size_t id) {
			auto target = id2typeinfo.find(id);
			if (target != id2typeinfo.end())
				return target->second;

			auto [iter, success] = id2typeinfo.try_emplace(id, id);
			assert(success);
			return iter->second;
		}

	private:
		std::unordered_map<size_t, TypeInfo> id2typeinfo;

		TypeInfoMngr() = default;
	};
}
