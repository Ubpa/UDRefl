#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <any>
#include <variant>
#include <functional>

namespace Ubpa::UDRefl {
	struct TypeInfo;
	
	template<typename Elem>
	struct ElemList {
		std::unordered_map<std::string, Elem> elems;
	};

	struct Attr {
		std::any value;
	};

	struct AttrList : ElemList<Attr> {};

	struct Field {
		struct NonStaticVar {
			std::function<std::any(void*)> getter;
			std::function<void(void*, std::any)> setter;
		};
		struct StaticVar {
			std::function<std::any()> getter;
			std::function<void(std::any)> setter;
		};
		struct NonStaticFunc {
			std::any func;
		};
		struct StaticFunc {
			std::any func;
		};
		using Value = std::variant<NonStaticVar, StaticVar, NonStaticFunc, StaticFunc>;
		Value value;
		AttrList attrs;

		template<typename T>
		static T& GetVar(void* p, size_t offset) {
			return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(p) + offset);
		}
	};

	struct FieldList : ElemList<Field> {};

	struct Base {
		TypeInfo* info;
		size_t offset;
		bool isVirtual{ false };
	};

	struct BaseList : ElemList<Base> {};

	struct TypeInfo {
		std::string name;
		FieldList fields;
		AttrList attrs;
		BaseList bases;
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
