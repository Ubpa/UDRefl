#pragma once

#include "Basic.h"
#include "IDRegistry.h"

#include <optional>
#include <span>

#define OBJECT_VIEW_DECLARE_OPERATOR(op, name) \
template<typename Arg>                         \
SharedObject operator op (Arg&& rhs) const

#define OBJECT_VIEW_DEFINE_CMP_OPERATOR(op, name)                                     \
template<typename Arg>                                                                \
bool operator op (const Arg& rhs) const {                                             \
    return static_cast<bool>(ADMInvoke(StrIDRegistry::MetaID::operator_##name, rhs)); \
}

#define OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(op, name)                            \
template<typename Arg>                                                             \
const ObjectView& operator op (Arg&& rhs) const {                                  \
	AInvoke<void>(StrIDRegistry::MetaID::operator_##name, std::forward<Arg>(rhs)); \
	return *this;                                                                  \
}

#define OBJECT_VIEW_DECLARE_CONTAINER(name) \
template<typename Arg>                      \
SharedObject name (Arg&& rhs) const

#define OBJECT_VIEW_DECLARE_CONTAINER_VARS(name) \
template<typename... Args>                       \
SharedObject name (Args&&... args) const

namespace Ubpa::UDRefl {
	class SharedObject;
	class ObjectView;

	template<typename T>
	struct IsObjectOrView;
	template<typename T>
	constexpr bool IsObjectOrView_v = IsObjectOrView<T>::value;
	template<typename T>
	concept NonObjectAndView = !IsObjectOrView_v<T>;

	class ObjectView {
	public:
		constexpr ObjectView() noexcept : ptr{ nullptr } {}
		constexpr ObjectView(std::nullptr_t) noexcept : ObjectView{} {}
		constexpr ObjectView(TypeID ID, void* ptr) noexcept : ID{ ID }, ptr{ ptr }{}
		explicit constexpr ObjectView(TypeID ID) noexcept : ObjectView{ ID, nullptr } {}
		template<typename T> requires
			std::negation_v<std::is_same<std::remove_cvref_t<T>, TypeID>>
			&& std::negation_v<std::is_same<std::remove_cvref_t<T>, std::nullptr_t>>
			&& NonObjectAndView<T>
		explicit ObjectView(T&& obj) noexcept : ObjectView{ TypeID_of<decltype(obj)>, const_cast<void*>(static_cast<const void*>(&obj)) } {}

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

		//////////////
		// ReflMngr //
		//////////////

		TypeInfo* GetTypeInfo() const;

		std::string_view TypeName() const;

		//
		// Cast
		/////////

		ObjectView StaticCast_DerivedToBase (TypeID baseID   ) const;
		ObjectView StaticCast_BaseToDerived (TypeID derivedID) const;
		ObjectView DynamicCast_BaseToDerived(TypeID derivedID) const;
		ObjectView StaticCast               (TypeID typeID   ) const;
		ObjectView DynamicCast              (TypeID typeID   ) const;

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

		// 'A' means auto, ObjectView/SharedObject will be transformed as ID + ptr
		template<typename T, typename... Args>
		T AInvoke(StrID methodID, Args&&... args) const;

		// 'A' means auto, ObjectView/SharedObject will be transformed as ID + ptr
		template<typename... Args>
		SharedObject AMInvoke(
			StrID methodID,
			std::pmr::memory_resource* rst_rsrc,
			Args&&... args) const;

		// 'A' means auto, ObjectView/SharedObject will be transformed as ID + ptr
		template<typename... Args>
		SharedObject ADMInvoke(
			StrID methodID,
			Args&&... args) const;

		//
		// Fields
		///////////

		// all
		ObjectView Var(StrID fieldID) const;

		// all, for diamond inheritance
		ObjectView Var(TypeID baseID, StrID fieldID) const;

		// self vars and all bases' vars
		void ForEachVar(const std::function<bool(TypeRef, FieldRef, ObjectView)>& func) const;

		// self owned vars and all bases' owned vars
		void ForEachOwnedVar(const std::function<bool(TypeRef, FieldRef, ObjectView)>& func) const;

		//
		// Algorithm
		//////////////

		std::vector<TypeID>                                    GetTypeIDs() const;
		std::vector<TypeRef>                                   GetTypes() const;
		std::vector<TypeFieldRef>                              GetTypeFields() const;
		std::vector<FieldRef>                                  GetFields() const;
		std::vector<TypeMethodRef>                             GetTypeMethods() const;
		std::vector<MethodRef>                                 GetMethods() const;
		std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> GetTypeFieldVars() const;
		std::vector<ObjectView>                                GetVars() const;
		std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> GetTypeFieldOwnedVars() const;
		std::vector<ObjectView>                                GetOwnedVars() const;

		std::optional<TypeID   > FindTypeID   (const std::function<bool(TypeID        )>& func) const;
		std::optional<TypeRef  > FindType     (const std::function<bool(TypeRef       )>& func) const;
		std::optional<FieldRef > FindField    (const std::function<bool(FieldRef      )>& func) const;
		std::optional<MethodRef> FindMethod   (const std::function<bool(MethodRef     )>& func) const;
		ObjectView           FindVar     (const std::function<bool(ObjectView)>& func) const;
		ObjectView           FindOwnedVar(const std::function<bool(ObjectView)>& func) const;

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

		ObjectView RemoveConst() const;
		ObjectView RemoveReference() const;
		ObjectView RemoveConstReference() const;

		ObjectView AddConst() const;
		ObjectView AddLValueReference() const;
		ObjectView AddLValueReferenceWeak() const;
		ObjectView AddRValueReference() const;
		ObjectView AddConstLValueReference() const;
		ObjectView AddConstRValueReference() const;


		//
		// Meta
		/////////

		OBJECT_VIEW_DECLARE_OPERATOR(+, add);
		OBJECT_VIEW_DECLARE_OPERATOR(-, sub);
		OBJECT_VIEW_DECLARE_OPERATOR(*, mul);
		OBJECT_VIEW_DECLARE_OPERATOR(/, div);
		OBJECT_VIEW_DECLARE_OPERATOR(%, mod);
		OBJECT_VIEW_DECLARE_OPERATOR(&, band);
		OBJECT_VIEW_DECLARE_OPERATOR(|, bor);
		OBJECT_VIEW_DECLARE_OPERATOR(^, bxor);

		OBJECT_VIEW_DEFINE_CMP_OPERATOR(<, lt);
		OBJECT_VIEW_DEFINE_CMP_OPERATOR(<=, le);
		OBJECT_VIEW_DEFINE_CMP_OPERATOR(>, gt);
		OBJECT_VIEW_DEFINE_CMP_OPERATOR(>=, ge);

		template<typename Arg> requires NonObjectAndView<std::decay_t<Arg>>
		const ObjectView& operator=(Arg&& rhs) const {
			AInvoke<void>(StrIDRegistry::MetaID::operator_assign, std::forward<Arg>(rhs));
			return *this;    
		}

		OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(+=, assign_add);
		OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(-=, assign_sub);
		OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(*=, assign_mul);
		OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(/=, assign_div);
		OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(%=, assign_mod);
		OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(&=, assign_band);
		OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(|=, assign_bor);
		OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(^=, assign_bxor);
		OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(<<=, assign_lshift);
		OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(>>=, assign_rshift);

		SharedObject operator++() const;
		SharedObject operator++(int) const;
		SharedObject operator--() const;
		SharedObject operator--(int) const;
		SharedObject operator+() const;
		SharedObject operator-() const;
		SharedObject operator~() const;
		SharedObject operator*() const;

		OBJECT_VIEW_DECLARE_OPERATOR([], subscript);
		OBJECT_VIEW_DECLARE_OPERATOR(->*, member_of_pointer);
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
		ObjectView tuple_get(std::size_t i) const;

		//
		// container
		//////////////

		OBJECT_VIEW_DECLARE_CONTAINER_VARS(assign);

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
		OBJECT_VIEW_DECLARE_CONTAINER(resize);
		void reserve(std::size_t n) const;
		void shrink_to_fit() const;

		// - element access

		OBJECT_VIEW_DECLARE_CONTAINER(at);
		SharedObject front() const;
		SharedObject back() const;
		SharedObject data() const;

		// - lookup

		OBJECT_VIEW_DECLARE_CONTAINER(count);
		OBJECT_VIEW_DECLARE_CONTAINER(find);
		OBJECT_VIEW_DECLARE_CONTAINER(lower_bound);
		OBJECT_VIEW_DECLARE_CONTAINER(upper_bound);
		OBJECT_VIEW_DECLARE_CONTAINER(equal_range);

		// - modifiers

		void clear() const;
		OBJECT_VIEW_DECLARE_CONTAINER_VARS(insert);
		OBJECT_VIEW_DECLARE_CONTAINER_VARS(insert_or_assign);
		OBJECT_VIEW_DECLARE_CONTAINER(erase);
		OBJECT_VIEW_DECLARE_CONTAINER(push_front);
		OBJECT_VIEW_DECLARE_CONTAINER(push_back);
		void pop_front() const;
		void pop_back() const;
		OBJECT_VIEW_DECLARE_CONTAINER(swap);
		OBJECT_VIEW_DECLARE_CONTAINER(merge);
		OBJECT_VIEW_DECLARE_CONTAINER(extract);

		// - observers

		SharedObject key_comp() const;
		SharedObject value_comp() const;
		SharedObject hash_function() const;
		SharedObject key_eq() const;
		SharedObject get_allocator() const;

	protected:
		TypeID ID;
		void* ptr; // if type is reference, ptr is a pointer of referenced object
	};
	
	class SharedObject : public ObjectView {
	public:
		//
		// Constructor
		////////////////

		using ObjectView::ObjectView;

		SharedObject(TypeID ID, SharedBuffer buffer) noexcept : ObjectView{ ID }, buffer{ std::move(buffer) } { ptr = buffer.get(); }

		template<typename T>
		SharedObject(TypeID ID, std::shared_ptr<T> buffer) noexcept : ObjectView{ ID, buffer.get() }, buffer{ std::move(buffer) } { }

		template<typename Deleter>
		SharedObject(ObjectView obj, Deleter d) noexcept : ObjectView{ obj }, buffer{ obj.GetPtr(), std::move(d) } {}
		template<typename U, typename Deleter, typename Alloc>
		SharedObject(ObjectView obj, Deleter d, Alloc alloc) noexcept : ObjectView{ obj }, buffer{ obj.GetPtr(), std::move(d), alloc } {}

		SharedObject(const SharedObject& obj) noexcept = default;
		SharedObject(SharedObject&& obj) noexcept = default;

		SharedObject& operator=(const SharedObject& rhs) noexcept = default;

		SharedObject& operator=(SharedObject&& rhs) noexcept = default;

		SharedObject& operator=(std::nullptr_t) noexcept {
			Reset();
			return *this;
		}

		// set pointer to nullptr
		void Reset() noexcept { ptr = nullptr; buffer.reset(); }

		SharedBuffer& GetBuffer() noexcept { return buffer; }
		const SharedBuffer& GetBuffer() const noexcept { return buffer; }

		long UseCount() const noexcept { return buffer.use_count(); }

		bool IsObjectView() const noexcept {
			return ptr && !buffer;
		}

		void Swap(SharedObject& rhs) noexcept {
			std::swap(ID, rhs.ID);
			std::swap(ptr, rhs.ptr);
			buffer.swap(rhs.buffer);
		}

	private:
		SharedBuffer buffer; // if type is reference, buffer is empty
	};
}

#undef OBJECT_VIEW_DECLARE_OPERATOR
#undef OBJECT_VIEW_DEFINE_CMP_OPERATOR
#undef OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR
#undef OBJECT_VIEW_DECLARE_CONTAINER
#undef OBJECT_VIEW_DECLARE_CONTAINER_VARS

#include "details/Object.inl"
