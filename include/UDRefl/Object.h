#pragma once

#include "Basic.h"
#include "IDRegistry.h"

#include <UContainer/Span.h>

#include <optional>

#define OBJECT_PTR_DECLARE_OPERATOR(op, name)                                              \
template<typename Arg>                                                                     \
SharedObject operator op (Arg rhs) const

#define OBJECT_PTR_DEFINE_CMP_OPERATOR(op, name)                                                              \
template<typename Arg>                                                                                        \
bool operator op (Arg rhs) const {                                                                            \
    return static_cast<bool>(ADMInvoke<Arg>(StrIDRegistry::MetaID::operator_##name, std::forward<Arg>(rhs))); \
}

#define OBJECT_PTR_DECLARE_CONTAINER(name) \
template<typename Arg>                     \
SharedObject name (Arg rhs) const

#define OBJECT_PTR_DECLARE_CONTAINER_VARS(name) \
template<typename... Args>                      \
SharedObject name (Args... args) const

#define SHARED_OBJECT_DECLARE_OPERATOR(op)                           \
template<typename Arg>                                               \
SharedObject operator op (Arg rhs) const

#define SHARED_OBJECT_DEFINE_OPERATOR(op)                            \
template<typename Arg>                                               \
SharedObject operator op (Arg rhs) const {                           \
    return AsObjectPtr()->operator op <Arg>(std::forward<Arg>(rhs)); \
}

#define SHARED_OBJECT_DEFINE_CMP_OPERATOR(op)                        \
template<typename Arg>                                               \
bool operator op (Arg rhs) const {                                   \
    return AsObjectPtr()->operator op <Arg>(std::forward<Arg>(rhs)); \
}

#define SHARED_OBJECT_DEFINE_UNARY_OPERATOR(op) \
SharedObject operator op () const {             \
    return AsObjectPtr()->operator op();        \
}

namespace Ubpa::UDRefl {
	class SharedObject;
	class SharedConstObject;
	class ObjectPtr;
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
		constexpr bool Is() const noexcept { return ID == TypeID_of<T>; }

		constexpr void Reset() noexcept { ptr = nullptr; }
		constexpr void Clear() noexcept { *this = ObjectPtrBase{}; }

		constexpr bool Valid() const noexcept { return ID.Valid() && ptr; }
		explicit operator bool() const noexcept { return ptr != nullptr ? (Is<bool>() ? *reinterpret_cast<bool*>(ptr) : true) : false; }

		//
		// ReflMngr
		/////////////

		TypeInfo* GetType() const noexcept;
		
		ConstObjectPtr StaticCast_DerivedToBase (TypeID baseID   ) const noexcept;
		ConstObjectPtr StaticCast_BaseToDerived (TypeID derivedID) const noexcept;
		ConstObjectPtr DynamicCast_BaseToDerived(TypeID derivedID) const noexcept;
		ConstObjectPtr StaticCast               (TypeID typeID   ) const noexcept;
		ConstObjectPtr DynamicCast              (TypeID typeID   ) const noexcept;

		InvocableResult IsInvocable(StrID methodID, Span<const TypeID> argTypeIDs = {}) const noexcept;

		InvokeResult Invoke(
			StrID methodID,
			void* result_buffer = nullptr,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr) const;

		template<typename... Args>
		InvocableResult IsInvocable(StrID methodID) const noexcept;

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

		// A means auto, ObjectPtr/SharedObject will be transform as ID + ptr
		template<typename... Args>
		SharedObject AMInvoke(
			StrID methodID,
			std::pmr::memory_resource* rst_rsrc,
			Args... args) const;

		// A means auto, ObjectPtr/SharedObject will be transform as ID + ptr
		template<typename... Args>
		SharedObject ADMInvoke(
			StrID methodID,
			Args... args) const;

		std::string_view TypeName() const noexcept;

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

		std::optional<TypeID   > FindTypeID(const std::function<bool(TypeID        )>& func) const;
		std::optional<TypeRef  > FindType  (const std::function<bool(TypeRef       )>& func) const;
		std::optional<FieldRef > FindField (const std::function<bool(FieldRef      )>& func) const;
		std::optional<MethodRef> FindMethod(const std::function<bool(MethodRef     )>& func) const;
		ConstObjectPtr           FindRVar  (const std::function<bool(ConstObjectPtr)>& func) const;

		DereferenceProperty GetDereferenceProperty() const;
		TypeID              DereferenceID() const;
		ObjectPtr           Dereference() const;
		ConstObjectPtr      DereferenceAsConst() const;

		//
		// Meta
		/////////

		OBJECT_PTR_DECLARE_OPERATOR(+, add);
		OBJECT_PTR_DECLARE_OPERATOR(-, sub);
		OBJECT_PTR_DECLARE_OPERATOR(*, mul);
		OBJECT_PTR_DECLARE_OPERATOR(/, div);
		OBJECT_PTR_DECLARE_OPERATOR(%, mod);
		OBJECT_PTR_DECLARE_OPERATOR(&, band);
		OBJECT_PTR_DECLARE_OPERATOR(|, bor);
		OBJECT_PTR_DECLARE_OPERATOR(^, bxor);

		OBJECT_PTR_DECLARE_OPERATOR([], subscript);
		OBJECT_PTR_DECLARE_OPERATOR(->*, member_of_pointer);

		OBJECT_PTR_DEFINE_CMP_OPERATOR(==, eq);
		OBJECT_PTR_DEFINE_CMP_OPERATOR(!=, ne);
		OBJECT_PTR_DEFINE_CMP_OPERATOR(< , lt);
		OBJECT_PTR_DEFINE_CMP_OPERATOR(<=, le);
		OBJECT_PTR_DEFINE_CMP_OPERATOR(> , gt);
		OBJECT_PTR_DEFINE_CMP_OPERATOR(>=, ge);
			
		SharedObject operator+() const;
		SharedObject operator-() const;
		SharedObject operator~() const;
		SharedObject operator[](std::size_t n) const;
		SharedObject operator*() const;

		template<typename... Args>
		SharedObject operator()(Args... args) const;

		template<typename T>
		T& operator>>(T& out) const {
			ADMInvoke<T&>(StrIDRegistry::MetaID::operator_rshift, out);
			return out;
		}

		//
		// container
		//////////////

		// - iterator

		SharedObject begin() const;
		SharedObject cbegin() const;
		SharedObject end() const;
		SharedObject cend() const;
		SharedObject rbegin() const;
		SharedObject crbegin() const;
		SharedObject rend() const;
		SharedObject crend() const;

		// - element access

		OBJECT_PTR_DECLARE_CONTAINER(at);
		SharedObject data() const;
		SharedObject front() const;
		SharedObject back() const;
		SharedObject empty() const;
		SharedObject size() const;
		//SharedObject max_size() const;
		SharedObject capacity() const;

		// - lookup

		OBJECT_PTR_DECLARE_CONTAINER(count);
		OBJECT_PTR_DECLARE_CONTAINER(find);
		OBJECT_PTR_DECLARE_CONTAINER(lower_bound);
		OBJECT_PTR_DECLARE_CONTAINER(upper_bound);
		OBJECT_PTR_DECLARE_CONTAINER(equal_range);

		// - observers

		SharedObject key_comp() const;
		SharedObject value_comp() const;
		SharedObject hash_function() const;
		SharedObject key_eq() const;
		SharedObject get_allocator() const;

	protected:
		template<typename T>
		auto* AsPtr() const noexcept {
			static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>);
			assert(ID.Is<T>());
			if constexpr (std::is_reference_v<T>)
				return reinterpret_cast<std::add_pointer_t<T>*>(ptr);
			else
				return reinterpret_cast<T*>(ptr);
		}

		template<typename T>
		decltype(auto) As() const noexcept {
			assert(ptr);
			auto* ptr = AsPtr<T>();
			if constexpr (std::is_reference_v<T>)
				return std::forward<T>(**ptr);
			else
				return *ptr;
		}

	protected:
		TypeID ID;
		void* ptr;
	};

	class ConstObjectPtr : public ObjectPtrBase {
	public:
		using ObjectPtrBase::ObjectPtrBase;

		constexpr ConstObjectPtr(TypeID ID, const void* ptr) noexcept : ObjectPtrBase{ ID, const_cast<void*>(ptr) } {}

		template<typename T, std::enable_if_t<!std::is_void_v<T>, int> = 0>
		constexpr ConstObjectPtr(const T* ptr) : ConstObjectPtr{ TypeID_of<std::remove_volatile_t<T>>, ptr } {}

		ConstObjectPtr(const SharedConstObject& obj) noexcept;

		constexpr const void* GetPtr() const noexcept { return ptr; }

		template<typename T>
		constexpr const T* AsPtr() const noexcept { return ObjectPtrBase::AsPtr<T>(); }
		template<typename T>
		constexpr const T& As() const noexcept { return ObjectPtrBase::As<T>(); }

		constexpr       ConstObjectPtr* operator->()       noexcept { return this; }
		constexpr const ConstObjectPtr* operator->() const noexcept { return this; }
	};

	class ObjectPtr : public ObjectPtrBase {
	public:
		using ObjectPtrBase::ObjectPtrBase;

		template<typename T, std::enable_if_t<!std::is_void_v<T>, int> = 0>
		constexpr ObjectPtr(std::remove_const_t<T>* ptr) : ObjectPtr{ TypeID_of<std::remove_volatile_t<T>>, ptr } {}

		constexpr void* GetPtr() const noexcept { return ptr; }

		using ObjectPtrBase::AsPtr;
		using ObjectPtrBase::As;

		constexpr operator ConstObjectPtr() const noexcept { return {ID, ptr}; }

		constexpr       ObjectPtr* operator->()       noexcept { return this; }
		constexpr const ObjectPtr* operator->() const noexcept { return this; }

		//
		// ReflMngr
		/////////////

		// variable
		// if &{const&{T}}, return nullptr
		ObjectPtr RWVar(StrID fieldID) const noexcept;
		// variable, for diamond inheritance
		// if &{const&{T}}, return nullptr
		ObjectPtr RWVar(TypeID baseID, StrID fieldID) const noexcept;

		ObjectPtr StaticCast_DerivedToBase (TypeID baseID)    const noexcept;
		ObjectPtr StaticCast_BaseToDerived (TypeID derivedID) const noexcept;
		ObjectPtr DynamicCast_BaseToDerived(TypeID derivedID) const noexcept;
		ObjectPtr StaticCast               (TypeID typeID)    const noexcept;
		ObjectPtr DynamicCast              (TypeID typeID)    const noexcept;

		InvocableResult IsInvocable(StrID methodID, Span<const TypeID> argTypeIDs = {}) const noexcept;

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
		InvocableResult IsInvocable(StrID methodID) const noexcept;

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

		// A means auto, ObjectPtr/SharedObject will be transform as ID + ptr
		template<typename... Args>
		SharedObject AMInvoke(
			StrID methodID,
			std::pmr::memory_resource* rst_rsrc,
			Args... args) const;

		// A means auto, ObjectPtr/SharedObject will be transform as ID + ptr
		template<typename... Args>
		SharedObject ADMInvoke(
			StrID methodID,
			Args... args) const;

		// self [r/w] vars and all bases' [r/w] vars
		void ForEachRWVar(const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const;

		std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> GetTypeFieldRWVars();
		std::vector<ObjectPtr>                                GetRWVars();

		ObjectPtr FindRWVar(const std::function<bool(ObjectPtr)>& func) const;

		//
		// Meta
		/////////
		
		OBJECT_PTR_DECLARE_OPERATOR(=, assign);
		OBJECT_PTR_DECLARE_OPERATOR(+=, assign_add);
		OBJECT_PTR_DECLARE_OPERATOR(-=, assign_sub);
		OBJECT_PTR_DECLARE_OPERATOR(*=, assign_mul);
		OBJECT_PTR_DECLARE_OPERATOR(/=, assign_div);
		OBJECT_PTR_DECLARE_OPERATOR(%=, assign_mod);
		OBJECT_PTR_DECLARE_OPERATOR(&=, assign_band);
		OBJECT_PTR_DECLARE_OPERATOR(|=, assign_bor);
		OBJECT_PTR_DECLARE_OPERATOR(^=, assign_bxor);
		OBJECT_PTR_DECLARE_OPERATOR(<<=, assign_lshift);
		OBJECT_PTR_DECLARE_OPERATOR(>>=, assign_rshift);

		OBJECT_PTR_DECLARE_OPERATOR([], subscript);
		OBJECT_PTR_DECLARE_OPERATOR(->*, member_of_pointer);
			
		SharedObject operator++() const;
		SharedObject operator++(int) const;
		SharedObject operator--() const;
		SharedObject operator--(int) const;
		SharedObject operator[](std::size_t n) const;
		SharedObject operator*() const;

		template<typename... Args>
		SharedObject operator()(Args... args) const;

		template<typename T>
		SharedObject operator<<(const T& in) const;

		//
		// container
		//////////////

		OBJECT_PTR_DECLARE_CONTAINER_VARS(assign);

		// - iterator

		SharedObject begin() const;
		SharedObject end() const;
		SharedObject rbegin() const;
		SharedObject rend() const;

		// - element access

		OBJECT_PTR_DECLARE_CONTAINER(at);
		SharedObject data() const;
		SharedObject front() const;
		SharedObject back() const;

		// - capacity

		OBJECT_PTR_DECLARE_CONTAINER(resize);
		void reserve(std::size_t n) const;
		void shrink_to_fit() const;

		// - modifiers

		void clear() const;
		OBJECT_PTR_DECLARE_CONTAINER_VARS(insert);
		OBJECT_PTR_DECLARE_CONTAINER_VARS(insert_or_assign);
		OBJECT_PTR_DECLARE_CONTAINER(erase);
		OBJECT_PTR_DECLARE_CONTAINER(push_front);
		OBJECT_PTR_DECLARE_CONTAINER(push_back);
		void pop_front() const;
		void pop_back() const;
		OBJECT_PTR_DECLARE_CONTAINER(swap);
		OBJECT_PTR_DECLARE_CONTAINER(merge);
		OBJECT_PTR_DECLARE_CONTAINER(extract);
		
		// - lookup

		OBJECT_PTR_DECLARE_CONTAINER(find);
		OBJECT_PTR_DECLARE_CONTAINER(lower_bound);
		OBJECT_PTR_DECLARE_CONTAINER(upper_bound);
		OBJECT_PTR_DECLARE_CONTAINER(equal_range);
	};

	static_assert(sizeof(ObjectPtr) == sizeof(ConstObjectPtr) && alignof(ObjectPtr) == alignof(ConstObjectPtr));

	constexpr ObjectPtr ConstCast(ConstObjectPtr ptr) noexcept {
		return { ptr.GetID(), const_cast<void*>(ptr.GetPtr()) };
	}
	
	// SharedBuffer + ID
	class SharedObjectBase {
	public:
		//
		// Constructor
		////////////////

		constexpr SharedObjectBase() noexcept = default;
		constexpr SharedObjectBase(std::nullptr_t) noexcept {}
		explicit constexpr SharedObjectBase(TypeID ID) noexcept : ID{ ID } {}

		SharedObjectBase(TypeID ID, const SharedBuffer& buffer) noexcept : ID{ ID }, buffer{ buffer } {}
		SharedObjectBase(TypeID ID, SharedBuffer&& buffer) noexcept : ID{ ID }, buffer{ std::move(buffer) } {}

		template<typename T>
		SharedObjectBase(TypeID ID, const std::shared_ptr<T>& buffer) noexcept : ID{ ID }, buffer{ buffer } {}
		template<typename T>
		SharedObjectBase(TypeID ID, std::shared_ptr<T>&& buffer) noexcept : ID{ ID }, buffer{ std::move(buffer) } {}

		template<typename Deleter>
		SharedObjectBase(ObjectPtr obj, Deleter d) noexcept : ID{ obj.GetID() }, buffer{ obj.GetPtr(), std::move(d) } {}
		template<typename U, typename Deleter, typename Alloc>
		SharedObjectBase(ObjectPtr obj, Deleter d, Alloc alloc) noexcept : ID{ obj.GetID() }, buffer{ obj.GetPtr(), std::move(d), alloc } {}

		//
		// Modifiers
		//////////////

		void Reset() noexcept {
			buffer.reset();
			ID.Reset();
		}

		//
		// Observers
		//////////////

		TypeID GetID() const noexcept { return ID; }

		long UseCount() const noexcept { return buffer.use_count(); }

		constexpr bool Valid() const noexcept { return ID.Valid() && buffer; }
		explicit operator bool() const noexcept { return ID ? static_cast<bool>(AsObjectPtr()) : false; }

		//
		// Meta
		/////////

		SHARED_OBJECT_DECLARE_OPERATOR(+);
		SHARED_OBJECT_DECLARE_OPERATOR(-);
		SHARED_OBJECT_DECLARE_OPERATOR(*);
		SHARED_OBJECT_DECLARE_OPERATOR(/);
		SHARED_OBJECT_DECLARE_OPERATOR(%);
		SHARED_OBJECT_DECLARE_OPERATOR(&);
		SHARED_OBJECT_DECLARE_OPERATOR(|);
		SHARED_OBJECT_DECLARE_OPERATOR(^);

		SHARED_OBJECT_DEFINE_CMP_OPERATOR(==)
		SHARED_OBJECT_DEFINE_CMP_OPERATOR(!=)
		SHARED_OBJECT_DEFINE_CMP_OPERATOR(>)
		SHARED_OBJECT_DEFINE_CMP_OPERATOR(>=)
		SHARED_OBJECT_DEFINE_CMP_OPERATOR(<)
		SHARED_OBJECT_DEFINE_CMP_OPERATOR(<=)

		SHARED_OBJECT_DECLARE_OPERATOR([]);
		SHARED_OBJECT_DECLARE_OPERATOR(->*);
			
		SharedObject operator+() const;
		SharedObject operator-() const;
		SharedObject operator~() const;

		SharedObject operator[](std::size_t n) const;
		SharedObject operator*() const;

		template<typename... Args>
		SharedObject operator()(Args... args) const;

		template<typename T>
		T& operator>>(T& out) const { return AsObjectPtr() >> out; }

		//
		// container
		//////////////

		SharedObject begin() const;
		SharedObject end() const;

	protected:
		void Swap(SharedObjectBase& rhs) noexcept {
			std::swap(ID, rhs.ID);
			buffer.swap(rhs.buffer);
		}

		void* GetPtr() const noexcept { return buffer.get(); }

		template<typename T>
		auto* AsPtr() const noexcept {
			static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>);
			assert(ID.Is<T>());
			if constexpr (std::is_reference_v<T>)
				return reinterpret_cast<const std::add_pointer_t<T>*>(GetPtr());
			else
				return reinterpret_cast<T*>(GetPtr());
		}

		template<typename T>
		decltype(auto) As() const noexcept {
			assert(GetPtr());
			auto* ptr = AsPtr<T>();
			if constexpr (std::is_reference_v<T>)
				return std::forward<T>(**ptr);
			else
				return *ptr;
		}

		TypeID ID;
		SharedBuffer buffer;

	private:
		ConstObjectPtr AsObjectPtr() const noexcept { return { ID, buffer.get() }; }
	};

	// SharedBuffer + ID
	class SharedConstObject : public SharedObjectBase {
	public:
		//
		// Constructor
		////////////////

		using SharedObjectBase::SharedObjectBase;

		SharedConstObject(const SharedConstObject& obj) noexcept : SharedObjectBase{ obj.ID, obj.buffer } {}
		SharedConstObject(SharedConstObject&& obj) noexcept : SharedObjectBase{ obj.ID, std::move(obj.buffer) } {}

		SharedConstObject(const SharedObject& obj);
		SharedConstObject(SharedObject&& obj) noexcept;

		SharedConstObject(TypeID ID, const SharedConstBuffer& buffer) noexcept : SharedObjectBase{ ID, std::const_pointer_cast<void>(buffer) } {}
		SharedConstObject(TypeID ID, SharedConstBuffer&& buffer) noexcept : SharedObjectBase{ ID, std::const_pointer_cast<void>(std::move(buffer)) } {}

		template<typename T>
		SharedConstObject(TypeID ID, const std::shared_ptr<const T>& buffer) noexcept : SharedObjectBase{ ID, std::const_pointer_cast<T>(buffer) } {}
		template<typename T>
		SharedConstObject(TypeID ID, std::shared_ptr<const T>&& buffer) noexcept : SharedObjectBase{ ID, std::const_pointer_cast<T>(std::move(buffer)) } {}

		template<typename U, typename Deleter>
		SharedConstObject(ConstObjectPtr obj, Deleter d) noexcept : SharedObjectBase{ ConstCast(obj), std::move(d) } {}
		template<typename U, typename Deleter, typename Alloc>
		SharedConstObject(ConstObjectPtr obj, Deleter d, Alloc alloc) noexcept : SharedObjectBase{ ConstCast(obj), std::move(d), alloc } {}

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

		void Swap(SharedConstObject& rhs) noexcept {
			SharedObjectBase::Swap(rhs);
		}

		//
		// Observers
		//////////////

		const SharedConstBuffer& GetBuffer() const noexcept { return reinterpret_cast<const SharedConstBuffer&>(buffer); }

		const void* GetPtr() const noexcept { return buffer.get(); }

		template<typename T>
		const auto* AsPtr() const noexcept {
			return SharedObjectBase::AsPtr<T>();
		}

		template<typename T>
		decltype(auto) As() const noexcept {
			return SharedObjectBase::As<T>();
		}

		ConstObjectPtr AsObjectPtr() const noexcept { return { ID, buffer.get() }; }

		ConstObjectPtr operator->() const noexcept { return AsObjectPtr(); }

		operator ConstObjectPtr() const noexcept { return AsObjectPtr(); }
	};

	// SharedBuffer + ID
	class SharedObject : public SharedObjectBase {
	public:
		//
		// Constructor
		////////////////

		using SharedObjectBase::SharedObjectBase;

		SharedObject(const SharedObject& obj) : SharedObjectBase{ obj.ID, obj.buffer } {}
		SharedObject(SharedObject && obj) noexcept : SharedObjectBase{ obj.ID, std::move(obj.buffer) } {}

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

		//
		// Modifiers
		//////////////

		void Swap(SharedObject& rhs) noexcept { SharedObjectBase::Swap(rhs); }

		//
		// Observers
		//////////////

		const SharedBuffer& GetBuffer() const noexcept { return buffer; }

		void* GetPtr() const noexcept { return buffer.get(); }

		using SharedObjectBase::AsPtr;
		using SharedObjectBase::As;
		
		SharedConstObject AsSharedConstObject() const noexcept { return { ID, buffer }; }
		ObjectPtr AsObjectPtr() const noexcept { return { ID, buffer.get() }; }
		ConstObjectPtr AsConstObjectPtr() const noexcept { return { ID, buffer.get() }; }

		ObjectPtr operator->() const noexcept { return AsObjectPtr(); }

		operator ObjectPtr() const noexcept { return AsObjectPtr(); }
		operator ConstObjectPtr() const noexcept { return AsConstObjectPtr(); }
		operator SharedConstObject() const noexcept { return { ID, buffer }; };

		//
		// Meta
		/////////

		using SharedObjectBase::operator*;
		
		SHARED_OBJECT_DEFINE_OPERATOR(=)
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
		SHARED_OBJECT_DEFINE_OPERATOR(->*)

		SHARED_OBJECT_DEFINE_UNARY_OPERATOR(++)
		SHARED_OBJECT_DEFINE_UNARY_OPERATOR(--)
		SharedObject operator++(int) const { return AsObjectPtr()++; }
		SharedObject operator--(int) const { return AsObjectPtr()--; }
		SharedObject operator[](std::size_t n) const { return AsObjectPtr()[n]; }
		SHARED_OBJECT_DEFINE_UNARY_OPERATOR(*)

		template<typename... Args>
		SharedObject operator()(Args... args) const {
			return AsObjectPtr()->operator()<Args...>(std::forward<Args>(args)...);
		}

		template<typename T>
		T& operator<<(const T& in) const { return AsObjectPtr() << in; }

		//
		// container
		//////////////

		SharedObject begin() const { return AsObjectPtr()->begin(); }
		SharedObject end() const { return AsObjectPtr()->end(); }
	};

	static_assert(sizeof(SharedObject) == sizeof(SharedConstObject) && alignof(SharedObject) == alignof(SharedConstObject));

	inline SharedObject ConstCast(const SharedConstObject& obj) {
		return { obj.GetID(), std::const_pointer_cast<void>(obj.GetBuffer()) };
	}

	template<typename T>
	constexpr auto Ptr(T&& p) noexcept {
		using U = std::remove_reference_t<T>;
		if constexpr (std::is_const_v<U>)
			return ConstObjectPtr{ TypeID_of<std::remove_cv_t<U>>, &p };
		else
			return ObjectPtr{ TypeID_of<std::remove_volatile_t<U>>, &p };
	}

	template<typename T>
	constexpr TypeID ArgID(T&& arg) noexcept {
		if constexpr (std::is_same_v<T, ObjectPtr> || std::is_same_v<T, SharedObject>)
			return arg.GetID();
		else {
			static_assert(!std::is_same_v<T, ConstObjectPtr> && !std::is_same_v<T, SharedConstObject>);
			return TypeID_of<T>;
		}
	}

	template<typename T>
	constexpr void* ArgPtr(T&& arg) noexcept {
		using U = std::remove_reference_t<T>;
		if constexpr (std::is_same_v<U, ObjectPtr> || std::is_same_v<U, SharedObject>)
			return arg.GetPtr();
		else {
			static_assert(!std::is_same_v<U, ConstObjectPtr> && !std::is_same_v<U, SharedConstObject>);
			return &arg;
		}
	}

	template<typename T>
	struct IsObjectOrPtr;
	template<typename T>
	constexpr bool IsObjectOrPtr_v = IsObjectOrPtr<T>::value;
}

#undef OBJECT_PTR_DECLARE_OPERATOR
#undef OBJECT_PTR_DEFINE_CMP_OPERATOR
#undef OBJECT_PTR_DECLARE_CONTAINER
#undef OBJECT_PTR_DECLARE_CONTAINER_VARS
#undef SHARED_OBJECT_DECLARE_OPERATOR
#undef SHARED_OBJECT_DEFINE_OPERATOR
#undef SHARED_OBJECT_DEFINE_CMP_OPERATOR
#undef SHARED_OBJECT_DEFINE_UNARY_OPERATOR

#include "details/Object.inl"
