#pragma once

#include "Basic.h"
#include "IDRegistry.h"

#include <optional>
#include <span>

#define OBJECT_PTR_DECLARE_OPERATOR(op, name) \
template<typename Arg>                        \
SharedObject operator op (Arg&& rhs) const

#define OBJECT_PTR_DEFINE_CMP_OPERATOR(op, name)                                      \
template<typename Arg>                                                                \
bool operator op (const Arg& rhs) const {                                             \
    return static_cast<bool>(ADMInvoke(StrIDRegistry::MetaID::operator_##name, rhs)); \
}

#define OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR(op, name)                             \
template<typename Arg>                                                             \
const ObjectPtr& operator op (Arg&& rhs) const {                                   \
	AInvoke<void>(StrIDRegistry::MetaID::operator_##name, std::forward<Arg>(rhs)); \
	return *this;                                                                  \
}

#define OBJECT_PTR_DECLARE_CONTAINER(name) \
template<typename Arg>                     \
SharedObject name (Arg&& rhs) const

#define OBJECT_PTR_DECLARE_CONTAINER_VARS(name) \
template<typename... Args>                      \
SharedObject name (Args&&... args) const

#define SHARED_OBJECT_DEFINE_OPERATOR(op)                \
template<typename Arg>                                   \
SharedObject operator op (Arg&& rhs) const {             \
    AsObjectPtr()->operator op (std::forward<Arg>(rhs)); \
    return *this;                                        \
}

#define SHARED_OBJECT_DEFINE_CMP_OPERATOR(op) \
template<typename Arg>                        \
bool operator op (const Arg& rhs) const {     \
    return AsObjectPtr()->operator op (rhs);  \
}

#define SHARED_OBJECT_DEFINE_UNARY_OPERATOR(op) \
SharedObject operator op () const {             \
    return AsObjectPtr()->operator op();        \
}

namespace Ubpa::UDRefl {
	class SharedObject;
	class ObjectPtr;

	template<typename T>
	struct IsObjectOrPtr;
	template<typename T>
	constexpr bool IsObjectOrPtr_v = IsObjectOrPtr<T>::value;
	template<typename T>
	concept NonObjectAndPtr = !IsObjectOrPtr<T>::value;

	class ObjectPtr {
	public:
		constexpr ObjectPtr() noexcept : ptr{ nullptr } {}
		constexpr ObjectPtr(std::nullptr_t) noexcept : ptr{ nullptr } {}
		constexpr ObjectPtr(TypeID ID, void* ptr) noexcept : ID{ ID }, ptr{ ptr }{}
		explicit constexpr ObjectPtr(TypeID ID) noexcept : ObjectPtr{ ID, nullptr } {}
		template<typename T> requires
			std::negation_v<std::is_same<std::remove_cvref_t<T>, TypeID>>
			&& std::negation_v<std::is_same<std::remove_cvref_t<T>, std::nullptr_t>>
			&& NonObjectAndPtr<T>
		explicit ObjectPtr(T&& obj) noexcept : ObjectPtr{ TypeID_of<decltype(obj)>, const_cast<void*>(static_cast<const void*>(&obj)) } {}

		constexpr TypeID GetTypeID() const noexcept { return ID; }
		void* GetPtr() const noexcept { return ptr; }

		constexpr bool Valid() const noexcept { return ID.Valid() && ptr; }
		explicit operator bool() const noexcept;

		template<typename T>
		auto* AsPtr() const noexcept {
			assert(ID.Is<T>());
			return reinterpret_cast<std::add_pointer_t<T>>(ptr);
		}

		template<typename T>
		decltype(auto) As() const noexcept {
			assert(ptr);
			auto* ptr = AsPtr<T>();
			if constexpr (std::is_reference_v<T>)
				return std::forward<T>(*ptr);
			else
				return *ptr;
		}

		constexpr       ObjectPtr* operator->()       noexcept { return this; }
		constexpr const ObjectPtr* operator->() const noexcept { return this; }

		//////////////
		// ReflMngr //
		//////////////

		TypeInfo* GetTypeInfo() const;

		std::string_view TypeName() const;

		//
		// Cast
		/////////

		ObjectPtr StaticCast_DerivedToBase (TypeID baseID   ) const;
		ObjectPtr StaticCast_BaseToDerived (TypeID derivedID) const;
		ObjectPtr DynamicCast_BaseToDerived(TypeID derivedID) const;
		ObjectPtr StaticCast               (TypeID typeID   ) const;
		ObjectPtr DynamicCast              (TypeID typeID   ) const;

		//
		// Invoke
		///////////

		InvocableResult IsInvocable(StrID methodID, std::span<const TypeID> argTypeIDs = {}) const;

		InvokeResult Invoke(
			StrID methodID,
			void* result_buffer = nullptr,
			std::span<const TypeID> argTypeIDs = {},
			ArgPtrBuffer argptr_buffer = nullptr) const;

		template<typename... Args>
		InvocableResult IsInvocable(StrID methodID) const;

		template<typename T>
		T InvokeRet(StrID methodID, std::span<const TypeID> argTypeIDs = {}, ArgPtrBuffer argptr_buffer = nullptr) const;

		template<typename... Args>
		InvokeResult InvokeArgs(StrID methodID, void* result_buffer, Args&&... args) const;

		template<typename T, typename... Args>
		T Invoke(StrID methodID, Args&&... args) const;

		SharedObject MInvoke(
			StrID methodID,
			std::span<const TypeID> argTypeIDs = {},
			ArgPtrBuffer argptr_buffer = nullptr,
			std::pmr::memory_resource* rst_rsrc = std::pmr::get_default_resource()) const;

		template<typename... Args>
		SharedObject MInvoke(
			StrID methodID,
			std::pmr::memory_resource* rst_rsrc,
			Args&&... args) const;

		template<typename... Args>
		SharedObject DMInvoke(
			StrID methodID,
			Args&&... args) const;

		// 'A' means auto, ObjectPtr/SharedObject will be transformed as ID + ptr
		template<typename T, typename... Args>
		T AInvoke(StrID methodID, Args&&... args) const;

		// 'A' means auto, ObjectPtr/SharedObject will be transformed as ID + ptr
		template<typename... Args>
		SharedObject AMInvoke(
			StrID methodID,
			std::pmr::memory_resource* rst_rsrc,
			Args&&... args) const;

		// 'A' means auto, ObjectPtr/SharedObject will be transformed as ID + ptr
		template<typename... Args>
		SharedObject ADMInvoke(
			StrID methodID,
			Args&&... args) const;

		//
		// Fields
		///////////

		// all
		ObjectPtr Var(StrID fieldID) const;

		// all, for diamond inheritance
		ObjectPtr Var(TypeID baseID, StrID fieldID) const;

		// self vars and all bases' vars
		void ForEachVar(const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const;

		// self owned vars and all bases' owned vars
		void ForEachOwnedVar(const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const;

		//
		// Algorithm
		//////////////

		std::vector<TypeID>                                        GetTypeIDs();
		std::vector<TypeRef>                                       GetTypes();
		std::vector<TypeFieldRef>                                  GetTypeFields();
		std::vector<FieldRef>                                      GetFields();
		std::vector<TypeMethodRef>                                 GetTypeMethods();
		std::vector<MethodRef>                                     GetMethods();
		std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> GetTypeFieldVars();
		std::vector<ObjectPtr>                                GetVars();
		std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> GetTypeFieldOwnedVars();
		std::vector<ObjectPtr>                                GetOwnedVars();

		std::optional<TypeID   > FindTypeID   (const std::function<bool(TypeID        )>& func) const;
		std::optional<TypeRef  > FindType     (const std::function<bool(TypeRef       )>& func) const;
		std::optional<FieldRef > FindField    (const std::function<bool(FieldRef      )>& func) const;
		std::optional<MethodRef> FindMethod   (const std::function<bool(MethodRef     )>& func) const;
		ObjectPtr           FindVar     (const std::function<bool(ObjectPtr)>& func) const;
		ObjectPtr           FindOwnedVar(const std::function<bool(ObjectPtr)>& func) const;

		bool ContainsBase          (TypeID baseID  ) const;
		bool ContainsField         (StrID  fieldID ) const;
		bool ContainsMethod        (StrID  methodID) const;
		bool ContainsVariableMethod(StrID  methodID) const;
		bool ContainsConstMethod   (StrID  methodID) const;
		bool ContainsStaticMethod  (StrID  methodID) const;

		//
		// Type
		/////////

		bool IsConst() const;
		bool IsReadOnly() const;
		bool IsReference() const;
		ConstReferenceMode GetConstReferenceMode() const;

		ObjectPtr RemoveConst() const;
		ObjectPtr RemoveReference() const;
		ObjectPtr RemoveConstReference() const;

		ObjectPtr AddConst() const;
		ObjectPtr AddLValueReference() const;
		ObjectPtr AddLValueReferenceWeak() const;
		ObjectPtr AddRValueReference() const;
		ObjectPtr AddConstLValueReference() const;
		ObjectPtr AddConstRValueReference() const;


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

		OBJECT_PTR_DEFINE_CMP_OPERATOR(<, lt);
		OBJECT_PTR_DEFINE_CMP_OPERATOR(<=, le);
		OBJECT_PTR_DEFINE_CMP_OPERATOR(>, gt);
		OBJECT_PTR_DEFINE_CMP_OPERATOR(>=, ge);

		template<typename Arg> requires NonObjectAndPtr<std::decay_t<Arg>>
		const ObjectPtr& operator=(Arg&& rhs) const {
			AInvoke<void>(StrIDRegistry::MetaID::operator_assign, std::forward<Arg>(rhs));
			return *this;    
		}

		OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR(+=, assign_add);
		OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR(-=, assign_sub);
		OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR(*=, assign_mul);
		OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR(/=, assign_div);
		OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR(%=, assign_mod);
		OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR(&=, assign_band);
		OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR(|=, assign_bor);
		OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR(^=, assign_bxor);
		OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR(<<=, assign_lshift);
		OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR(>>=, assign_rshift);

		SharedObject operator++() const;
		SharedObject operator++(int) const;
		SharedObject operator--() const;
		SharedObject operator--(int) const;
		SharedObject operator+() const;
		SharedObject operator-() const;
		SharedObject operator~() const;
		SharedObject operator*() const;

		OBJECT_PTR_DECLARE_OPERATOR([], subscript);
		OBJECT_PTR_DECLARE_OPERATOR(->*, member_of_pointer);
		SharedObject operator[](std::size_t n) const;

		template<typename... Args>
		SharedObject operator()(Args&&... args) const;

		template<typename T>
		T& operator>>(T& out) const {
			ADMInvoke(StrIDRegistry::MetaID::operator_rshift, out);
			return out;
		}

		template<typename T>
		SharedObject operator<<(T&& in) const;

		//
		// Tuple
		//////////

		std::size_t tuple_size() const;
		ObjectPtr tuple_get(std::size_t i) const;

		//
		// container
		//////////////

		OBJECT_PTR_DECLARE_CONTAINER_VARS(assign);

		// - iterator

		SharedObject cbegin() const;
		SharedObject cend() const;
		SharedObject crbegin() const;
		SharedObject crend() const;

		SharedObject begin() const;
		SharedObject end() const;
		SharedObject rbegin() const;
		SharedObject rend() const;

		// - capacity

		SharedObject empty() const;
		SharedObject size() const;
		//SharedObject max_size() const;
		SharedObject capacity() const;
		SharedObject bucket_count() const;
		OBJECT_PTR_DECLARE_CONTAINER(resize);
		void reserve(std::size_t n) const;
		void shrink_to_fit() const;

		// - element access

		OBJECT_PTR_DECLARE_CONTAINER(at);
		SharedObject front() const;
		SharedObject back() const;
		SharedObject data() const;

		// - lookup

		OBJECT_PTR_DECLARE_CONTAINER(count);
		OBJECT_PTR_DECLARE_CONTAINER(find);
		OBJECT_PTR_DECLARE_CONTAINER(lower_bound);
		OBJECT_PTR_DECLARE_CONTAINER(upper_bound);
		OBJECT_PTR_DECLARE_CONTAINER(equal_range);

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

		// - observers

		SharedObject key_comp() const;
		SharedObject value_comp() const;
		SharedObject hash_function() const;
		SharedObject key_eq() const;
		SharedObject get_allocator() const;

	private:
		TypeID ID;
		void* ptr; // if type is reference, ptr is a pointer of referenced object
	};
	
	// SharedBuffer + ID
	class SharedObject {
	public:
		//
		// Constructor
		////////////////

		constexpr SharedObject() noexcept = default;
		constexpr SharedObject(std::nullptr_t) noexcept {}
		explicit constexpr SharedObject(TypeID ID) noexcept : ID{ ID } {}

		SharedObject(TypeID ID, SharedBuffer buffer) noexcept : ID{ ID }, buffer{ std::move(buffer) } {}

		template<typename T>
		SharedObject(TypeID ID, std::shared_ptr<T> buffer) noexcept : ID{ ID }, buffer{ std::move(buffer) } {}

		template<typename Deleter>
		SharedObject(ObjectPtr obj, Deleter d) noexcept : ID{ obj.GetTypeID() }, buffer{ obj.GetPtr(), std::move(d) } {}
		template<typename U, typename Deleter, typename Alloc>
		SharedObject(ObjectPtr obj, Deleter d, Alloc alloc) noexcept : ID{ obj.GetTypeID() }, buffer{ obj.GetPtr(), std::move(d), alloc } {}

		SharedObject(const SharedObject& obj) noexcept : SharedObject{ obj.ID, obj.buffer } {}
		SharedObject(SharedObject&& obj) noexcept : SharedObject{ obj.ID, std::move(obj.buffer) } {}

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

		// set pointer to nullptr
		void Reset() noexcept { buffer.reset(); }

		// Reset and set ID to invalid
		void Clear() noexcept { *this = SharedObject{}; }

		TypeID GetTypeID() const noexcept { return ID; }

		SharedBuffer& GetBuffer() noexcept { return buffer; }
		const SharedBuffer& GetBuffer() const noexcept { return buffer; }

		long UseCount() const noexcept { return buffer.use_count(); }

		constexpr bool Valid() const noexcept { return ID.Valid() && static_cast<bool>(buffer); }
		explicit operator bool() const noexcept { return static_cast<bool>(AsObjectPtr()); }

		void Swap(SharedObject& rhs) noexcept {
			std::swap(ID, rhs.ID);
			buffer.swap(rhs.buffer);
		}

		void* GetPtr() const noexcept { return buffer.get(); }

		template<typename T>
		auto* AsPtr() const noexcept {
			assert(ID.Is<T>());
			return reinterpret_cast<std::add_pointer_t<T>>(GetPtr());
		}

		template<typename T>
		decltype(auto) As() const noexcept {
			assert(GetPtr());
			auto* ptr = AsPtr<T>();
			if constexpr (std::is_reference_v<T>)
				return std::forward<T>(*ptr);
			else
				return *ptr;
		}

		ObjectPtr AsObjectPtr() const noexcept { return { ID, buffer.get() }; }
		operator ObjectPtr() const noexcept { return AsObjectPtr(); }
		ObjectPtr operator->() const noexcept { return AsObjectPtr(); }

		//////////////
		// ReflMngr //
		//////////////

		//
		// Meta
		/////////

		SHARED_OBJECT_DEFINE_OPERATOR(+);
		SHARED_OBJECT_DEFINE_OPERATOR(-);
		SHARED_OBJECT_DEFINE_OPERATOR(*);
		SHARED_OBJECT_DEFINE_OPERATOR(/);
		SHARED_OBJECT_DEFINE_OPERATOR(%);
		SHARED_OBJECT_DEFINE_OPERATOR(&);
		SHARED_OBJECT_DEFINE_OPERATOR(|);
		SHARED_OBJECT_DEFINE_OPERATOR(^);

		SHARED_OBJECT_DEFINE_CMP_OPERATOR(>)
		SHARED_OBJECT_DEFINE_CMP_OPERATOR(>=)
		SHARED_OBJECT_DEFINE_CMP_OPERATOR(<)
		SHARED_OBJECT_DEFINE_CMP_OPERATOR(<=)

		template<typename Arg> requires NonObjectAndPtr<std::decay_t<Arg>>
		SharedObject operator=(Arg&& rhs) const {
			return AsObjectPtr()->operator=(std::forward<Arg>(rhs));
		}

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

		SHARED_OBJECT_DEFINE_UNARY_OPERATOR(++)
		SHARED_OBJECT_DEFINE_UNARY_OPERATOR(--)
		SharedObject operator++(int) const { return AsObjectPtr()++; }
		SharedObject operator--(int) const { return AsObjectPtr()--; }
		SHARED_OBJECT_DEFINE_UNARY_OPERATOR(+)
		SHARED_OBJECT_DEFINE_UNARY_OPERATOR(-)
		SHARED_OBJECT_DEFINE_UNARY_OPERATOR(~)
		SHARED_OBJECT_DEFINE_UNARY_OPERATOR(*)

		SHARED_OBJECT_DEFINE_OPERATOR([])
		SHARED_OBJECT_DEFINE_OPERATOR(->*)
		SharedObject operator[](std::size_t n) const { return AsObjectPtr()[n]; }

		template<typename... Args>
		SharedObject operator()(Args&&... args) const {
			return AsObjectPtr()->operator()<Args&&...>(std::forward<Args>(args)...);
		}

		template<typename T>
		T& operator>>(T& out) const { return AsObjectPtr() >> out; }

		template<typename T>
		SharedObject operator<<(T&& in) const { return AsObjectPtr() << std::forward<T>(in); }

		//
		// container
		//////////////

		SharedObject begin() const;
		SharedObject end() const;

	private:
		TypeID ID;
		SharedBuffer buffer; // if type is reference, ptr is a pointer of referenced object
	};
}

#undef OBJECT_PTR_DECLARE_OPERATOR
#undef OBJECT_PTR_DEFINE_CMP_OPERATOR
#undef OBJECT_PTR_DEFINE_ASSIGN_OP_OPERATOR
#undef OBJECT_PTR_DECLARE_CONTAINER
#undef OBJECT_PTR_DECLARE_CONTAINER_VARS
#undef SHARED_OBJECT_DEFINE_OPERATOR
#undef SHARED_OBJECT_DEFINE_CMP_OPERATOR
#undef SHARED_OBJECT_DEFINE_UNARY_OPERATOR

#include "details/Object.inl"
