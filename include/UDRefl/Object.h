#pragma once

#include "Basic.h"
#include "IDRegistry.h"

#include <optional>
#include <span>

#define OBJECT_VIEW_DECLARE_OPERATOR(op, name) \
template<typename Arg>                         \
SharedObject operator op (Arg&& rhs) const

#define OBJECT_VIEW_DEFINE_CMP_OPERATOR(op, name)                                    \
template<typename Arg>                                                               \
bool operator op (const Arg& rhs) const {                                            \
    return static_cast<bool>(ADMInvoke(NameIDRegistry::Meta::operator_##name, rhs)); \
}

#define OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(op, name)                           \
template<typename Arg>                                                            \
ObjectView operator op (Arg&& rhs) const {                                        \
    if(GetType().IsReadOnly())                                                    \
        return *this;                                                             \
    AInvoke<void>(NameIDRegistry::Meta::operator_##name, std::forward<Arg>(rhs)); \
    return AddLValueReference();                                                  \
}

#define OBJECT_VIEW_DECLARE_CONTAINER(name) \
template<typename Arg>                      \
SharedObject name (Arg&& rhs) const

#define OBJECT_VIEW_DECLARE_CONTAINER_VARS(name) \
template<typename... Args>                       \
SharedObject name (Args&&... args) const

namespace Ubpa::UDRefl {
	class ObjectView {
	public:
		constexpr ObjectView() noexcept : ptr{ nullptr } {}
		constexpr ObjectView(std::nullptr_t) noexcept : ObjectView{} {}
		constexpr ObjectView(Type type, void* ptr) noexcept : type{ type }, ptr{ ptr }{}
		constexpr ObjectView(Type type) noexcept : ObjectView{ type, nullptr } {}
		template<typename T> requires
			std::negation_v<std::is_same<std::remove_cvref_t<T>, Type>>
			&& std::negation_v<std::is_same<std::remove_cvref_t<T>, std::nullptr_t>>
			&& NonObjectAndView<T>
		constexpr explicit ObjectView(T&& obj) noexcept : ObjectView{ Type_of<decltype(obj)>, const_cast<void*>(static_cast<const void*>(&obj)) } {}

		constexpr Type GetType() const noexcept { return type; }
		constexpr void* GetPtr() const noexcept { return ptr; }

		explicit operator bool() const noexcept;

		template<typename T>
		constexpr auto* AsPtr() const noexcept {
			assert(type.Is<T>());
			return reinterpret_cast<std::add_pointer_t<T>>(ptr);
		}

		template<typename T>
		constexpr decltype(auto) As() const noexcept {
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

		//
		// Cast
		/////////

		ObjectView StaticCast_DerivedToBase (Type base   ) const;
		ObjectView StaticCast_BaseToDerived (Type derived) const;
		ObjectView DynamicCast_BaseToDerived(Type derived) const;
		ObjectView StaticCast               (Type type   ) const;
		ObjectView DynamicCast              (Type type   ) const;

		//
		// Invoke
		///////////

		InvocableResult IsInvocable(Name method_name, std::span<const Type> argTypes = {}, MethodFlag flag = MethodFlag::All) const;

		InvokeResult Invoke(
			Name method_name,
			void* result_buffer = nullptr,
			std::span<const Type> argTypes = {},
			ArgPtrBuffer argptr_buffer = nullptr,
			MethodFlag flag = MethodFlag::All) const;

		template<typename... Args>
		InvocableResult IsInvocable(Name method_name, MethodFlag flag = MethodFlag::All) const;

		template<typename T>
		T InvokeRet(Name method_name, std::span<const Type> argTypes = {}, ArgPtrBuffer argptr_buffer = nullptr, MethodFlag flag = MethodFlag::All) const;

		template<typename... Args>
		InvokeResult InvokeArgs(Name method_name, void* result_buffer, Args&&... args) const;

		template<typename T, typename... Args>
		T Invoke(Name method_name, Args&&... args) const;

		SharedObject MInvoke(
			Name method_name,
			std::pmr::memory_resource* rst_rsrc,
			std::span<const Type> argTypes = {},
			ArgPtrBuffer argptr_buffer = nullptr,
			MethodFlag flag = MethodFlag::All) const;

		SharedObject DMInvoke(
			Name method_name,
			std::span<const Type> argTypes = {},
			ArgPtrBuffer argptr_buffer = nullptr,
			MethodFlag flag = MethodFlag::All) const;

		template<typename... Args>
		SharedObject MInvoke(
			Name method_name,
			std::pmr::memory_resource* rst_rsrc,
			MethodFlag flag,
			Args&&... args) const;

		template<typename... Args>
		SharedObject DMInvoke(
			Name method_name,
			Args&&... args) const;

		// 'A' means auto, ObjectView/SharedObject will be transformed as type + ptr
		template<typename T, typename... Args>
		T AInvoke(Name method_name, Args&&... args) const;

		// 'A' means auto, ObjectView/SharedObject will be transformed as type + ptr
		template<typename... Args>
		SharedObject AMInvoke(
			Name method_name,
			std::pmr::memory_resource* rst_rsrc,
			MethodFlag flag,
			Args&&... args) const;

		// 'A' means auto, ObjectView/SharedObject will be transformed as type + ptr
		template<typename... Args>
		SharedObject ADMInvoke(
			Name method_name,
			Args&&... args) const;

		//
		// Fields
		///////////

		ObjectView Var(Name field_name, FieldFlag flag = FieldFlag::All) const;
		// for diamond inheritance
		ObjectView Var(Type base, Name field_name, FieldFlag flag = FieldFlag::All) const;

		// self vars and all bases' vars
		void ForEachVar(const std::function<bool(TypeRef, FieldRef, ObjectView)>& func, FieldFlag flag = FieldFlag::All) const;

		//
		// Algorithm
		//////////////

		std::vector<TypeRef>                                   GetTypes        () const;
		std::vector<TypeFieldRef>                              GetTypeFields   (FieldFlag flag = FieldFlag::All) const;
		std::vector<FieldRef>                                  GetFields       (FieldFlag flag = FieldFlag::All) const;
		std::vector<TypeMethodRef>                             GetTypeMethods  (MethodFlag  flag = MethodFlag ::All) const;
		std::vector<MethodRef>                                 GetMethods      (MethodFlag  flag = MethodFlag ::All) const;
		std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> GetTypeFieldVars(FieldFlag flag = FieldFlag::All) const;
		std::vector<ObjectView>                                GetVars         (FieldFlag flag = FieldFlag::All) const;

		std::optional<TypeRef  > FindType  (const std::function<bool(TypeRef   )>& func) const;
		std::optional<FieldRef > FindField (const std::function<bool(FieldRef  )>& func, FieldFlag flag = FieldFlag::All) const;
		std::optional<MethodRef> FindMethod(const std::function<bool(MethodRef )>& func, MethodFlag  flag = MethodFlag ::All) const;
		ObjectView               FindVar   (const std::function<bool(ObjectView)>& func, FieldFlag flag = FieldFlag::All) const;

		bool ContainsBase  (Type base       ) const;
		bool ContainsField (Name field_name , FieldFlag flag = FieldFlag::All) const;
		bool ContainsMethod(Name method_name, MethodFlag  flag = MethodFlag ::All) const;

		//
		// Type
		/////////

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

		OBJECT_VIEW_DEFINE_CMP_OPERATOR(< , lt);
		OBJECT_VIEW_DEFINE_CMP_OPERATOR(<=, le);
		OBJECT_VIEW_DEFINE_CMP_OPERATOR(> , gt);
		OBJECT_VIEW_DEFINE_CMP_OPERATOR(>=, ge);

		template<typename Arg> requires NonObjectAndView<std::decay_t<Arg>>
		ObjectView operator=(Arg&& rhs) const {
			AInvoke<void>(NameIDRegistry::Meta::operator_assign, std::forward<Arg>(rhs));
			return AddLValueReference();    
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
			ADMInvoke(NameIDRegistry::Meta::operator_rshift, out);
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
		Type type;
		void* ptr; // if type is reference, ptr is a pointer of referenced object
	};
	
	class SharedObject : public ObjectView {
	public:
		using ObjectView::ObjectView;
	
		SharedObject(Type type, SharedBuffer buffer) noexcept : ObjectView{ type }, buffer{ std::move(buffer) } { ptr = buffer.get(); }
		constexpr explicit SharedObject(ObjectView obj) noexcept : ObjectView{ obj } { }
		
		template<typename T>
		SharedObject(Type type, std::shared_ptr<T> buffer) noexcept : ObjectView{ type, buffer.get() }, buffer{ std::move(buffer) } { }

		template<typename Deleter>
		SharedObject(ObjectView obj, Deleter d) noexcept : ObjectView{ obj }, buffer{ obj.GetPtr(), std::move(d) } {}
		template<typename U, typename Deleter, typename Alloc>
		SharedObject(ObjectView obj, Deleter d, Alloc alloc) noexcept : ObjectView{ obj }, buffer{ obj.GetPtr(), std::move(d), alloc } {}

		// set pointer to nullptr
		void Reset() noexcept { ptr = nullptr; buffer.reset(); }

		SharedBuffer& GetBuffer() noexcept { return buffer; }
		const SharedBuffer& GetBuffer() const noexcept { return buffer; }

		long UseCount() const noexcept { return buffer.use_count(); }

		bool IsObjectView() const noexcept { return ptr && !buffer; }

		void Swap(SharedObject& rhs) noexcept {
			std::swap(type, rhs.type);
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
