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

		template<typename T>
		T& Var(size_t offset) {
			return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(ptr) + offset);
		}

	private:
		size_t id;
		void* ptr;
	};

	struct Attr {
		std::any value;
	};
	using AttrList = std::map<std::string, Attr, std::less<>>;

	struct Field {
		struct NonStaticVar {
			template<typename T>
			static NonStaticVar Init(size_t offset) {
				return {
					std::function{[=](Object obj)->std::any {
						return obj.Var<T>(offset);
					}},
					std::function{[=](Object obj, std::any value) {
						obj.Var<T>(offset) = std::any_cast<T>(value);
					}}
				};
			}

			std::function<std::any(Object)> get;
			std::function<void(Object, std::any)> set;
		};
		struct StaticVar {
			std::function<std::any()> get;
			std::function<void(std::any)> set;
		};
		using Funcs = std::vector<std::any>;
		using Value = std::variant<NonStaticVar, StaticVar, Funcs>;

		static constexpr const char default_constructor[] = "_default_constructor";
		static constexpr const char copy_constructor[] = "_copy_constructor";
		static constexpr const char move_constructor[] = "_move_constructor";
		static constexpr const char destructor[] = "_destructor";

		Value value;
		AttrList attrs;
	};
	
	using FieldList = std::map<std::string, Field, std::less<>>;

	struct Base {
		TypeInfo* info;
		size_t offset;
		bool isVirtual{ false };
	};

	using BaseList = std::map<std::string, Base, std::less<>>;

	struct TypeInfo {
		std::string name;

		size_t size;
		size_t alignment;

		BaseList bases;

		AttrList attrs;
		FieldList fields;

		template<typename Ret, typename... Args>
		Ret Call(std::string_view name, Args... args) {
			static_assert(std::is_void_v<Ret> || std::is_constructible_v<Ret>);

			using Func = std::function<Ret(Args...)>;
			auto& funcs = std::get<Field::Funcs>(fields.find(name)->second.value);
			for (auto& func : funcs) {
				if (func.type() == typeid(Func))
					return std::any_cast<Func>(func)(std::forward<Args>(args)...);
			}
			assert("arguments' types are matching failure with functions" && false);
			if constexpr(!std::is_void_v<Ret>)
				return {};
		}

		Object DefaultConstruct() {
			return Call<Object>(Field::default_constructor);
		}

		Object CopyConstruct(Object src) {
			return Call<Object, Object>(Field::copy_constructor, src);
		}

		Object MoveConstruct(Object src) {
			return Call<Object, Object>(Field::move_constructor, src);
		}

		void Destruct(Object p) {
			return Call<void, Object>(Field::destructor, p);
		}
	};

	class TypeInfoMngr {
	public:
		static TypeInfoMngr& Instance() {
			static TypeInfoMngr instance;
			return instance;
		}

		TypeInfo& GetTypeInfo(size_t id) {
			return id2typeinfo[id];
		}

	private:
		std::unordered_map<size_t, TypeInfo> id2typeinfo;

		TypeInfoMngr() = default;
	};
}
