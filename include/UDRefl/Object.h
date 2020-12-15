#pragma once

#include "SharedBuffer.h"
#include "Util.h"
#include "IDRegistry.h"

#include <UTemplate/TypeID.h>
#include <UContainer/Span.h>

#include <cassert>
#include <functional>
#include <memory>

#define OBJECT_PTR_DEFINE_OPERATOR(op, name)                                              \
template<typename Arg>                                                                    \
SharedObject operator##op (Arg rhs) const {                                               \
    return DMInvoke<Arg>(StrIDRegistry::MetaID::operator_##name, std::forward<Arg>(rhs)); \
}

#define SHARED_OBJECT_DEFINE_OPERATOR(op)                            \
template<typename Arg>                                               \
SharedObject operator##op (Arg rhs) const {                          \
    return AsObjectPtr()->operator##op<Arg>(std::forward<Arg>(rhs)); \
}

namespace Ubpa::UDRefl {
	struct InvokeResult {
		bool success{ false };
		TypeID resultID;
		Destructor destructor;

		template<typename T>
		T Move(void* result_buffer);

		constexpr bool IsVoid() const noexcept {
			return resultID.Is<void>();
		}

		constexpr operator bool() const noexcept { return success; }
	};

	class SharedObject;
	class SharedConstObject;
	struct TypeInfo;
	struct FieldInfo;
	struct MethodInfo;

	struct TypeRef {
		TypeID ID;
		TypeInfo& info;
	};

	struct FieldRef {
		StrID ID;
		FieldInfo& info;
	};

	struct MethodRef {
		StrID ID;
		MethodInfo& info;
	};

	struct TypeFieldRef {
		TypeRef type;
		FieldRef field;
	};

	struct TypeMethodRef {
		TypeRef type;
		MethodRef method;
	};

	class ConstObjectPtr;

	class ObjectPtrBase {
	public:
		constexpr ObjectPtrBase() noexcept : ptr{ nullptr } {}
		constexpr ObjectPtrBase(std::nullptr_t) noexcept : ObjectPtrBase{} {}
		constexpr ObjectPtrBase(TypeID ID, std::nullptr_t) noexcept : ID{ ID }, ptr{ nullptr } {}
		constexpr ObjectPtrBase(TypeID ID, void* ptr) noexcept : ID{ ID }, ptr{ ptr }{}
		explicit constexpr ObjectPtrBase(TypeID ID) noexcept : ObjectPtrBase{ ID, nullptr } {}

		constexpr TypeID GetID() const noexcept { return ID; }

		template<typename T>
		constexpr bool Is() const noexcept { return ID == TypeID::of<T>; }

		constexpr void Reset() noexcept { ptr = nullptr; }
		constexpr void Clear() noexcept { *this = ObjectPtrBase{}; }

		constexpr operator bool() const noexcept { return ptr != nullptr; }

		//
		// ReflMngr
		/////////////

		// all
		ConstObjectPtr RVar(StrID fieldID) const noexcept;

		// all, for diamond inheritance
		ConstObjectPtr RVar(TypeID baseID, StrID fieldID) const noexcept;

		// self [r] vars and all bases' [r] vars
		void ForEachRVar(const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const;

		std::vector<TypeID>                                        GetTypeIDs();
		std::vector<TypeRef>                                       GetTypes();
		std::vector<TypeFieldRef>                                  GetTypeFields();
		std::vector<FieldRef>                                      GetFields();
		std::vector<TypeMethodRef>                                 GetTypeMethods();
		std::vector<MethodRef>                                     GetMethods();
		std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> GetTypeFieldRVars();
		std::vector<ConstObjectPtr>                                GetRVars();

	protected:
		template<typename T>
		constexpr T* AsPtr() const noexcept {
			assert(Is<T>());
			return reinterpret_cast<T*>(ptr);
		}

		template<typename T>
		constexpr T& As() const noexcept { assert(ptr); return *AsPtr<T>(); }

	protected:
		TypeID ID;
		void* ptr;
	};

	class ConstObjectPtr : public ObjectPtrBase {
	public:
		using ObjectPtrBase::ObjectPtrBase;

		constexpr ConstObjectPtr(TypeID ID, const void* ptr) noexcept : ObjectPtrBase{ ID, const_cast<void*>(ptr) } {}

		template<typename T, std::enable_if_t<!std::is_void_v<T>, int> = 0>
		constexpr ConstObjectPtr(const volatile T* ptr) : ConstObjectPtr{ TypeID::of<T>, ptr } {}

		ConstObjectPtr(const SharedConstObject& obj) noexcept;

		constexpr const void* GetPtr() const noexcept { return ptr; }

		template<typename T>
		constexpr const T* AsPtr() const noexcept { return ObjectPtrBase::AsPtr<T>(); }
		template<typename T>
		constexpr const T& As() const noexcept { return ObjectPtrBase::As<T>(); }

		constexpr operator const void* () const noexcept { return ptr; }

		constexpr       ConstObjectPtr* operator->()       noexcept { return this; }
		constexpr const ConstObjectPtr* operator->() const noexcept { return this; }

		//
		// ReflMngr
		/////////////
		
		ConstObjectPtr StaticCast_DerivedToBase (TypeID baseID   ) const noexcept;
		ConstObjectPtr StaticCast_BaseToDerived (TypeID derivedID) const noexcept;
		ConstObjectPtr DynamicCast_BaseToDerived(TypeID derivedID) const noexcept;
		ConstObjectPtr StaticCast               (TypeID typeID   ) const noexcept;
		ConstObjectPtr DynamicCast              (TypeID typeID   ) const noexcept;

		bool IsInvocable(StrID methodID, Span<const TypeID> argTypeIDs = {}) const noexcept;

		InvokeResult Invoke(
			StrID methodID,
			void* result_buffer = nullptr,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr) const;

		template<typename... Args>
		bool IsInvocable(StrID methodID) const noexcept;

		template<typename T>
		T InvokeRet(StrID methodID, Span<const TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;

		template<typename... Args>
		InvokeResult InvokeArgs(StrID methodID, void* result_buffer, Args... args) const;

		template<typename T, typename... Args>
		T Invoke(StrID methodID, Args... args) const;

		SharedObject MInvoke(
			StrID methodID,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			std::pmr::memory_resource* rst_rsrc = std::pmr::get_default_resource()) const;

		template<typename... Args>
		SharedObject MInvoke(
			StrID methodID,
			std::pmr::memory_resource* rst_rsrc,
			Args... args) const;

		template<typename... Args>
		SharedObject DMInvoke(
			StrID methodID,
			Args... args) const;

		//
		// Meta
		/////////

		OBJECT_PTR_DEFINE_OPERATOR(+, add)
		OBJECT_PTR_DEFINE_OPERATOR(-, minus)
		OBJECT_PTR_DEFINE_OPERATOR(*, mul)
		OBJECT_PTR_DEFINE_OPERATOR(/, div)
		OBJECT_PTR_DEFINE_OPERATOR(%, mod)
		OBJECT_PTR_DEFINE_OPERATOR(&, band)
		OBJECT_PTR_DEFINE_OPERATOR(|, bor)
		OBJECT_PTR_DEFINE_OPERATOR(^, bxor)
		OBJECT_PTR_DEFINE_OPERATOR(<<, lshift)
		OBJECT_PTR_DEFINE_OPERATOR(>>, rshift)
		
		OBJECT_PTR_DEFINE_OPERATOR(+=, assign_add)
		OBJECT_PTR_DEFINE_OPERATOR(-=, assign_minus)
		OBJECT_PTR_DEFINE_OPERATOR(*=, assign_mul)
		OBJECT_PTR_DEFINE_OPERATOR(/=, assign_div)
		OBJECT_PTR_DEFINE_OPERATOR(%=, assign_mod)
		OBJECT_PTR_DEFINE_OPERATOR(&=, assign_band)
		OBJECT_PTR_DEFINE_OPERATOR(|=, assign_bor)
		OBJECT_PTR_DEFINE_OPERATOR(^=, assign_bxor)
		OBJECT_PTR_DEFINE_OPERATOR(<<=, assign_lshift)
		OBJECT_PTR_DEFINE_OPERATOR(>>=, assign_rshift)

		OBJECT_PTR_DEFINE_OPERATOR([], subscript)

		template<typename... Args>
		SharedObject operator()(Args... args) const {
			return DMInvoke<Args...>(StrIDRegistry::MetaID::operator_call,
				std::forward<Args>(args)...);
		}
	};

	class ObjectPtr : public ObjectPtrBase {
	public:
		using ObjectPtrBase::ObjectPtrBase;

		template<typename T, std::enable_if_t<!std::is_void_v<T>, int> = 0>
		constexpr ObjectPtr(std::add_volatile_t<std::remove_cv_t<T>>* ptr) : ObjectPtr{ TypeID::of<T>, ptr } {}

		constexpr void* GetPtr() const noexcept { return ptr; }

		using ObjectPtrBase::AsPtr;
		using ObjectPtrBase::As;

		constexpr operator void*() const noexcept { return ptr; }
		constexpr operator ConstObjectPtr() const noexcept { return {ID, ptr}; }

		constexpr       ObjectPtr* operator->()       noexcept { return this; }
		constexpr const ObjectPtr* operator->() const noexcept { return this; }

		//
		// ReflMngr
		/////////////

		// variable
		ObjectPtr      RWVar(StrID fieldID) const noexcept;
		// variable, for diamond inheritance
		ObjectPtr      RWVar(TypeID baseID, StrID fieldID) const noexcept;

		ObjectPtr StaticCast_DerivedToBase (TypeID baseID)    const noexcept;
		ObjectPtr StaticCast_BaseToDerived (TypeID derivedID) const noexcept;
		ObjectPtr DynamicCast_BaseToDerived(TypeID derivedID) const noexcept;
		ObjectPtr StaticCast               (TypeID typeID)    const noexcept;
		ObjectPtr DynamicCast              (TypeID typeID)    const noexcept;

		bool IsInvocable(StrID methodID, Span<const TypeID> argTypeIDs = {}) const noexcept;

		InvokeResult Invoke(
			StrID methodID,
			void* result_buffer = nullptr,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr) const;

		SharedObject MInvoke(
			StrID methodID,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			std::pmr::memory_resource* rst_rsrc = std::pmr::get_default_resource()) const;

		template<typename... Args>
		bool IsInvocable(StrID methodID) const noexcept;

		template<typename T>
		T InvokeRet(StrID methodID, Span<const TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;

		template<typename... Args>
		InvokeResult InvokeArgs(StrID methodID, void* result_buffer, Args... args) const;

		template<typename T, typename... Args>
		T Invoke(StrID methodID, Args... args) const;

		template<typename... Args>
		SharedObject MInvoke(
			StrID methodID,
			std::pmr::memory_resource* rst_rsrc,
			Args... args) const;

		template<typename... Args>
		SharedObject DMInvoke(
			StrID methodID,
			Args... args) const;

		// self [r/w] vars and all bases' [r/w] vars
		void ForEachRWVar(const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const;

		std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> GetTypeFieldRWVars();
		std::vector<ObjectPtr>                                GetRWVars();

		//
		// Meta
		/////////

		OBJECT_PTR_DEFINE_OPERATOR(+, add)
		OBJECT_PTR_DEFINE_OPERATOR(-, minus)
		OBJECT_PTR_DEFINE_OPERATOR(*, mul)
		OBJECT_PTR_DEFINE_OPERATOR(/, div)
		OBJECT_PTR_DEFINE_OPERATOR(%, mod)
		OBJECT_PTR_DEFINE_OPERATOR(&, band)
		OBJECT_PTR_DEFINE_OPERATOR(|, bor)
		OBJECT_PTR_DEFINE_OPERATOR(^, bxor)
		OBJECT_PTR_DEFINE_OPERATOR(<<, lshift)
		OBJECT_PTR_DEFINE_OPERATOR(>>, rshift)
		
		OBJECT_PTR_DEFINE_OPERATOR(+=, assign_add)
		OBJECT_PTR_DEFINE_OPERATOR(-=, assign_minus)
		OBJECT_PTR_DEFINE_OPERATOR(*=, assign_mul)
		OBJECT_PTR_DEFINE_OPERATOR(/=, assign_div)
		OBJECT_PTR_DEFINE_OPERATOR(%=, assign_mod)
		OBJECT_PTR_DEFINE_OPERATOR(&=, assign_band)
		OBJECT_PTR_DEFINE_OPERATOR(|=, assign_bor)
		OBJECT_PTR_DEFINE_OPERATOR(^=, assign_bxor)
		OBJECT_PTR_DEFINE_OPERATOR(<<=, assign_lshift)
		OBJECT_PTR_DEFINE_OPERATOR(>>=, assign_rshift)

		OBJECT_PTR_DEFINE_OPERATOR([], subscript)

		template<typename... Args>
		SharedObject operator()(Args... args) const {
			return DMInvoke<Args...>(StrIDRegistry::MetaID::operator_call,
				std::forward<Args>(args)...);
		}
	};

	static_assert(sizeof(ObjectPtr) == sizeof(ConstObjectPtr) && alignof(ObjectPtr) == alignof(ConstObjectPtr));

	using SharedBuffer = std::shared_ptr<void>;
	using SharedConstBuffer = std::shared_ptr<const void>;

	// SharedBuffer + ID
	class SharedConstObject {
	public:
		//
		// Constructor
		////////////////

		constexpr SharedConstObject() noexcept = default;
		SharedConstObject(const SharedConstObject& obj) : ID{ obj.ID }, buffer{ obj.buffer } {}
		SharedConstObject(SharedConstObject&& obj) noexcept : ID{ obj.ID }, buffer{ std::move(obj.buffer) } {}

		constexpr SharedConstObject(std::nullptr_t) noexcept {}
		explicit constexpr SharedConstObject(TypeID ID) noexcept : ID{ ID } {}

		SharedConstObject(TypeID ID, const SharedConstBuffer& buffer) noexcept : ID{ ID }, buffer{ buffer } {}
		SharedConstObject(TypeID ID, SharedConstBuffer&& buffer) noexcept : ID{ ID }, buffer{ std::move(buffer) } {}

		template<typename U, typename Deleter>
		SharedConstObject(ObjectPtr obj, Deleter&& d) noexcept : ID{ obj.GetID() }, buffer{ obj.GetPtr(), std::forward<Deleter>(d) } {}
		template<typename U, typename Deleter, typename Alloc>
		SharedConstObject(ObjectPtr obj, Deleter&& d, Alloc&& alloc) noexcept : ID{ obj.GetID() }, buffer{ obj.GetPtr(), std::forward<Deleter>(d), std::forward<Alloc>(alloc) } {}

		//
		// Assign
		///////////

		SharedConstObject& operator=(const SharedConstObject& rhs) noexcept {
			ID = rhs.ID;
			buffer = rhs.buffer;
			return *this;
		}

		SharedConstObject& operator=(SharedConstObject&& rhs) noexcept {
			SharedConstObject(std::move(rhs)).Swap(*this);
			return *this;
		}

		SharedConstObject& operator=(std::nullptr_t) noexcept {
			Reset();
			return *this;
		}

		//
		// Modifiers
		//////////////

		void Reset() noexcept {
			buffer.reset();
			ID.Reset();
		}

		void Swap(SharedConstObject& rhs) noexcept {
			std::swap(ID, rhs.ID);
			buffer.swap(rhs.buffer);
		}

		//
		// Observers
		//////////////

		TypeID GetID() const noexcept { return ID; }

		SharedConstBuffer        GetBuffer()      &  noexcept { return buffer; }
		const SharedConstBuffer& GetBuffer() const&  noexcept { return buffer; }
		SharedConstBuffer        GetBuffer()      && noexcept { return std::move(buffer); }

		const void* GetPtr() const noexcept { return buffer.get(); }

		template<typename T>
		const T* AsPtr() const noexcept {
			assert(ID.Is<T>());
			return reinterpret_cast<const T*>(GetPtr());
		}

		template<typename T>
		const T& As() const noexcept {
			assert(GetPtr());
			return *AsPtr<T>();
		}

		ConstObjectPtr AsObjectPtr() const noexcept { return { ID, buffer.get() }; }

		ConstObjectPtr operator->() const noexcept { return AsObjectPtr(); }

		long UseCount() const noexcept { return buffer.use_count(); }

		operator bool() const noexcept { return ID && static_cast<bool>(buffer); }
		operator ConstObjectPtr() const noexcept { return AsObjectPtr(); }

		//
		// Meta
		/////////

		SHARED_OBJECT_DEFINE_OPERATOR(+)
		SHARED_OBJECT_DEFINE_OPERATOR(-)
		SHARED_OBJECT_DEFINE_OPERATOR(*)
		SHARED_OBJECT_DEFINE_OPERATOR(/)
		SHARED_OBJECT_DEFINE_OPERATOR(%)
		SHARED_OBJECT_DEFINE_OPERATOR(&)
		SHARED_OBJECT_DEFINE_OPERATOR(|)
		SHARED_OBJECT_DEFINE_OPERATOR(^)
		SHARED_OBJECT_DEFINE_OPERATOR(<<)
		SHARED_OBJECT_DEFINE_OPERATOR(>>)
		
		SHARED_OBJECT_DEFINE_OPERATOR(+=)
		SHARED_OBJECT_DEFINE_OPERATOR(-=)
		SHARED_OBJECT_DEFINE_OPERATOR(*=)
		SHARED_OBJECT_DEFINE_OPERATOR(/=)
		SHARED_OBJECT_DEFINE_OPERATOR(%=)
		SHARED_OBJECT_DEFINE_OPERATOR(&=)
		SHARED_OBJECT_DEFINE_OPERATOR(|=)
		SHARED_OBJECT_DEFINE_OPERATOR(^=)
		SHARED_OBJECT_DEFINE_OPERATOR(<<=)
		SHARED_OBJECT_DEFINE_OPERATOR(>>=)

		SHARED_OBJECT_DEFINE_OPERATOR([])

		template<typename... Args>
		SharedObject operator()(Args... args) const {
			return AsObjectPtr()->operator()<Args...>(std::forward<Args>(args)...);
		}

	private:
		TypeID ID;
		SharedConstBuffer buffer;
	};

	// SharedBuffer + ID
	class SharedObject {
	public:
		//
		// Constructor
		////////////////

		constexpr SharedObject() noexcept = default;
		SharedObject(const SharedObject & obj) : ID{ obj.ID }, buffer{ obj.buffer } {}
		SharedObject(SharedObject && obj) noexcept : ID{ obj.ID }, buffer{ std::move(obj.buffer) } {}

		constexpr SharedObject(std::nullptr_t) noexcept {}
		explicit constexpr SharedObject(TypeID ID) noexcept : ID{ ID } {}

		SharedObject(TypeID ID, const SharedBuffer& buffer) noexcept : ID{ ID }, buffer{ buffer } {}
		SharedObject(TypeID ID, SharedBuffer&& buffer) noexcept : ID{ ID }, buffer{ std::move(buffer) } {}

		template<typename Deleter>
		SharedObject(ObjectPtr obj, Deleter&& d) noexcept : ID{ obj.GetID() }, buffer{ obj.GetPtr(), std::forward<Deleter>(d) } {}
		template<typename U, typename Deleter, typename Alloc>
		SharedObject(ObjectPtr obj, Deleter&& d, Alloc&& alloc) noexcept : ID{ obj.GetID() }, buffer{ obj.GetPtr(), std::forward<Deleter>(d), std::forward<Alloc>(alloc) } {}

		//
		// Assign
		///////////

		SharedObject& operator=(const SharedObject& rhs) noexcept {
			ID = rhs.ID;
			buffer = rhs.buffer;
			return *this;
		}

		SharedObject& operator=(SharedObject&& rhs) noexcept {
			SharedObject(std::move(rhs)).Swap(*this);
			return *this;
		}

		SharedObject& operator=(std::nullptr_t) noexcept {
			Reset();
			return *this;
		}

		//
		// Modifiers
		//////////////

		void Reset() noexcept {
			buffer.reset();
			ID.Reset();
		}

		void Swap(SharedObject& rhs) noexcept {
			std::swap(ID, rhs.ID);
			buffer.swap(rhs.buffer);
		}

		//
		// Observers
		//////////////

		TypeID GetID() const noexcept { return ID; }

		SharedBuffer        GetBuffer() & noexcept { return buffer; }
		const SharedBuffer& GetBuffer() const& noexcept { return buffer; }
		SharedBuffer        GetBuffer() && noexcept { return std::move(buffer); }

		void* GetPtr() const noexcept { return buffer.get(); }

		template<typename T>
		T* AsPtr() const noexcept { return reinterpret_cast<T*>(GetPtr()); }

		template<typename T>
		T& As() const noexcept { assert(GetPtr()); return *AsPtr<T>(); }
		
		ObjectPtr AsObjectPtr() const noexcept { return { ID, buffer.get() }; }

		ObjectPtr operator->() const noexcept { return AsObjectPtr(); }

		long UseCount() const noexcept { return buffer.use_count(); }

		operator bool() const noexcept { return ID && static_cast<bool>(buffer); }
		operator ObjectPtr() const noexcept { return AsObjectPtr(); }
		operator SharedConstObject() const noexcept { return { ID, SharedConstBuffer{buffer} }; };

		//
		// Meta
		/////////

		SHARED_OBJECT_DEFINE_OPERATOR(+)
		SHARED_OBJECT_DEFINE_OPERATOR(-)
		SHARED_OBJECT_DEFINE_OPERATOR(*)
		SHARED_OBJECT_DEFINE_OPERATOR(/)
		SHARED_OBJECT_DEFINE_OPERATOR(%)
		SHARED_OBJECT_DEFINE_OPERATOR(&)
		SHARED_OBJECT_DEFINE_OPERATOR(|)
		SHARED_OBJECT_DEFINE_OPERATOR(^)
		SHARED_OBJECT_DEFINE_OPERATOR(<<)
		SHARED_OBJECT_DEFINE_OPERATOR(>>)
		
		SHARED_OBJECT_DEFINE_OPERATOR(+=)
		SHARED_OBJECT_DEFINE_OPERATOR(-=)
		SHARED_OBJECT_DEFINE_OPERATOR(*=)
		SHARED_OBJECT_DEFINE_OPERATOR(/=)
		SHARED_OBJECT_DEFINE_OPERATOR(%=)
		SHARED_OBJECT_DEFINE_OPERATOR(&=)
		SHARED_OBJECT_DEFINE_OPERATOR(|=)
		SHARED_OBJECT_DEFINE_OPERATOR(^=)
		SHARED_OBJECT_DEFINE_OPERATOR(<<=)
		SHARED_OBJECT_DEFINE_OPERATOR(>>=)

		SHARED_OBJECT_DEFINE_OPERATOR([])

		template<typename... Args>
		SharedObject operator()(Args... args) const {
			return AsObjectPtr()->operator()<Args...>(std::forward<Args>(args)...);
		}

	private:
		TypeID ID;
		SharedBuffer buffer;
	};

	template<typename T>
	constexpr auto Ptr(T&& p) noexcept {
		using U = std::remove_reference_t<T>;
		if constexpr (std::is_const_v<U>)
			return ConstObjectPtr{ TypeID::of<std::remove_cv_t<U>>, &p };
		else
			return ObjectPtr{ TypeID::of<std::remove_volatile_t<U>>, &p };
	}
}

#undef OBJECT_PTR_DEFINE_OPERATOR
#undef SHARED_OBJECT_DEFINE_OPERATOR

#include "details/Object.inl"
