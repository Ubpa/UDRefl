#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <any>
#include <variant>
#include <functional>
#include <vector>

namespace Ubpa::UDRefl {
	struct TypeInfo;

	template<typename T>
	static T& Get(void* p, size_t offset) {
		return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(p) + offset);
	}

	struct Attr {
		std::any value;
	};
	using AttrList = std::unordered_map<std::string, Attr>;

	struct Field {
		struct NonStaticVar {
			template<typename T>
			static NonStaticVar Init(size_t offset) {
				return {
					std::function{[=](void* p)->std::any {
						return Get<T>(p, offset);
					}},
					std::function{[=](void* p, std::any value) {
						Get<T>(p, offset) = std::any_cast<T>(value);
					}}
				};
			}

			std::function<std::any(void*)> getter;
			std::function<void(void*, std::any)> setter;
		};
		struct StaticVar {
			std::function<std::any()> getter;
			std::function<void(std::any)> setter;
		};
		using Funcs = std::vector<std::any>;
		using Value = std::variant<NonStaticVar, StaticVar, Funcs>;
		Value value;
		AttrList attrs;
	};
	
	using FieldList = std::unordered_map<std::string, Field>;

	struct Base {
		TypeInfo* info;
		size_t offset;
		bool isVirtual{ false };
	};

	using BaseList = std::unordered_map<std::string, Attr>;

	struct TypeInfo {
		std::string name;

		size_t size;
		size_t alignment;

		BaseList bases;

		AttrList attrs;
		FieldList fields;
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
