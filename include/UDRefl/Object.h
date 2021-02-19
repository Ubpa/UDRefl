#pragma once

#include "Basic.h"
#include "IDRegistry.h"

//#include <span>

#define OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(op, name)                            \
template<typename Arg>                                                             \
ObjectView operator op (Arg&& rhs) const {                                         \
    if(GetType().IsReadOnly())                                                     \
        return {};                                                                 \
    ABInvoke<void>(NameIDRegistry::Meta::operator_##name, std::forward<Arg>(rhs)); \
    return AddLValueReference();                                                   \
}

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
		constexpr explicit ObjectView(T&& obj) noexcept
			: ObjectView{ Type_of<decltype(obj)>, const_cast<void*>(static_cast<const void*>(&obj)) } {}

		constexpr const Type& GetType() const noexcept { return type; }
		constexpr void* const& GetPtr() const noexcept { return ptr; }

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

		Type IsInvocable(Name method_name, std::span<const Type> argTypes = {}, MethodFlag flag = MethodFlag::All) const;

		Type BInvoke(
			Name method_name,
			void* result_buffer = nullptr,
			std::span<const Type> argTypes = {},
			ArgPtrBuffer argptr_buffer = nullptr,
			MethodFlag flag = MethodFlag::All) const;

		SharedObject MInvoke(
			Name method_name,
			std::pmr::memory_resource* rst_rsrc,
			std::pmr::memory_resource* temp_args_rsrc,
			std::span<const Type> argTypes = {},
			ArgPtrBuffer argptr_buffer = nullptr,
			MethodFlag flag = MethodFlag::All) const;

		SharedObject Invoke(
			Name method_name,
			std::span<const Type> argTypes = {},
			ArgPtrBuffer argptr_buffer = nullptr,
			MethodFlag flag = MethodFlag::All) const;

		// -- template --

		template<typename... Args>
		Type IsInvocable(Name method_name, MethodFlag flag = MethodFlag::All) const;

		template<typename T>
		T BInvokeRet(Name method_name, std::span<const Type> argTypes = {}, ArgPtrBuffer argptr_buffer = nullptr, MethodFlag flag = MethodFlag::All) const;

		template<typename T, typename... Args>
		T BInvoke(Name method_name, Args&&... args) const;

		template<typename... Args>
		SharedObject MInvoke(
			Name method_name,
			std::pmr::memory_resource* rst_rsrc,
			std::pmr::memory_resource* temp_args_rsrc,
			MethodFlag flag,
			Args&&... args) const;

		template<typename... Args>
		SharedObject Invoke(
			Name method_name,
			Args&&... args) const;

		template<typename T, typename... Args>
		T ABInvoke(Name method_name, Args&&... args) const;

		template<typename... Args>
		SharedObject AMInvoke(
			Name method_name,
			std::pmr::memory_resource* rst_rsrc,
			std::pmr::memory_resource* temp_args_rsrc,
			MethodFlag flag,
			Args&&... args) const;

		template<typename... Args>
		SharedObject AInvoke(
			Name method_name,
			Args&&... args) const;

		//
		// Var
		////////

		ObjectView Var(Name field_name, FieldFlag flag = FieldFlag::All) const;
		// for diamond inheritance
		ObjectView Var(Type base, Name field_name, FieldFlag flag = FieldFlag::All) const;

		//
		// Algorithm
		//////////////
		//
		// - only contains APIs with ObjectView in ReflMngr
		//

		void ForEachVar(const std::function<bool(InfoTypePair, InfoFieldPair, ObjectView)>& func, FieldFlag flag = FieldFlag::All) const;
		std::vector<std::tuple<InfoTypePair, InfoFieldPair, ObjectView>> GetTypeFieldVars(FieldFlag flag = FieldFlag::All) const;
		std::vector<ObjectView> GetVars(FieldFlag flag = FieldFlag::All) const;
		ObjectView FindVar(const std::function<bool(ObjectView)>& func, FieldFlag  flag = FieldFlag::All) const;

		//
		// Type
		/////////

		ObjectView RemoveConst() const;
		ObjectView RemoveLValueReference() const;
		ObjectView RemoveRValueReference() const;
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

		template<typename T> SharedObject operator+(T&& rhs) const;
		template<typename T> SharedObject operator-(T&& rhs) const;
		template<typename T> SharedObject operator*(T&& rhs) const;
		template<typename T> SharedObject operator/(T&& rhs) const;
		template<typename T> SharedObject operator%(T&& rhs) const;
		template<typename T> SharedObject operator&(T&& rhs) const;
		template<typename T> SharedObject operator|(T&& rhs) const;
		template<typename T> SharedObject operator^(T&& rhs) const;

		template<typename T> bool operator< (const T& rhs) const { return ABInvoke<bool>(NameIDRegistry::Meta::operator_lt, rhs); }
		template<typename T> bool operator<=(const T& rhs) const { return ABInvoke<bool>(NameIDRegistry::Meta::operator_le, rhs); }
		template<typename T> bool operator> (const T& rhs) const { return ABInvoke<bool>(NameIDRegistry::Meta::operator_gt, rhs); }
		template<typename T> bool operator>=(const T& rhs) const { return ABInvoke<bool>(NameIDRegistry::Meta::operator_ge, rhs); }

		template<typename Arg> requires NonObjectAndView<std::decay_t<Arg>>
		ObjectView operator=(Arg&& rhs) const {
			ABInvoke<void>(NameIDRegistry::Meta::operator_assign, std::forward<Arg>(rhs));
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

		template<typename T> SharedObject operator[](T&& rhs) const;
		SharedObject operator[](std::size_t n) const;

		template<typename... Args>
		SharedObject operator()(Args&&... args) const;

		template<typename T> T& operator>>(T& out) const;
		template<typename T> SharedObject operator<<(T&& in) const;
		
		////////////////////////
		// General Containers //
		////////////////////////

		ContainerType get_container_type() const;

		//
		// Tuple
		//////////

		std::size_t tuple_size() const { return BInvoke<std::size_t>(NameIDRegistry::Meta::tuple_size); }
		ObjectView tuple_get(std::size_t i) const { return BInvoke<ObjectView>(NameIDRegistry::Meta::tuple_get, std::move(i)); }
		ObjectView tuple_get(Type type) const { return BInvoke<ObjectView>(NameIDRegistry::Meta::tuple_get, std::move(type)); }
		Type tuple_element(std::size_t i) const { return BInvoke<Type>(NameIDRegistry::Meta::tuple_element, std::move(i)); }

		//
		// Variant
		////////////

		std::size_t variant_index() const { return BInvoke<std::size_t>(NameIDRegistry::Meta::variant_index); }
		std::size_t variant_size() const { return BInvoke<std::size_t>(NameIDRegistry::Meta::variant_size); }
		bool variant_holds_alternative(Type type) const { return BInvoke<bool>(NameIDRegistry::Meta::variant_holds_alternative, std::move(type)); }
		ObjectView variant_get(std::size_t i) const { return BInvoke<ObjectView>(NameIDRegistry::Meta::variant_get, std::move(i)); }
		ObjectView variant_get(Type type) const { return BInvoke<ObjectView>(NameIDRegistry::Meta::variant_get, std::move(type)); }
		Type variant_alternative(std::size_t i) const { return BInvoke<Type>(NameIDRegistry::Meta::variant_alternative, std::move(i)); }
		ObjectView variant_visit_get() const { return BInvoke<ObjectView>(NameIDRegistry::Meta::variant_visit_get); }

		//
		// Optional
		/////////////

		bool optional_has_value() const { return BInvoke<bool>(NameIDRegistry::Meta::optional_has_value); }
		ObjectView optional_value() const { return BInvoke<ObjectView>(NameIDRegistry::Meta::optional_value); }
		void optional_reset() const { BInvoke<void>(NameIDRegistry::Meta::optional_reset); }

		//
		// Iterator
		/////////////

		template<typename T> void advance(T&& arg) const { ABInvoke<void>(NameIDRegistry::Meta::iterator_advance, std::forward<T>(arg)); };
		template<typename T> std::size_t distance(T&& arg) const { return ABInvoke<std::size_t>(NameIDRegistry::Meta::iterator_distance, std::forward<T>(arg)); };
		template<typename T> SharedObject next(T&& arg) const;
		template<typename T> SharedObject prev(T&& arg) const;
		SharedObject next() const;
		SharedObject prev() const;

		//
		// Container
		//////////////

		template<typename... Args> void assign(Args&&... args) const { ABInvoke<void>(NameIDRegistry::Meta::container_assign, std::forward<Args>(args)...); };

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

		bool empty() const { return BInvoke<bool>(NameIDRegistry::Meta::container_empty); }
		std::size_t size() const { return BInvoke<std::size_t>(NameIDRegistry::Meta::container_size); }
		std::size_t capacity() const { return BInvoke<std::size_t>(NameIDRegistry::Meta::container_capacity); }
		std::size_t bucket_count() const { return BInvoke<std::size_t>(NameIDRegistry::Meta::container_bucket_count); }
		template<typename T> void resize(T&& arg) const { ABInvoke<void>(NameIDRegistry::Meta::container_resize, std::forward<T>(arg)); };
		void reserve(std::size_t n) const { BInvoke<void>(NameIDRegistry::Meta::container_reserve, std::move(n)); }
		void shrink_to_fit() const { BInvoke<void>(NameIDRegistry::Meta::container_shrink_to_fit); }

		// - element access

		template<typename T> SharedObject at(T&& rhs) const;
		SharedObject front() const;
		SharedObject back() const;
		SharedObject top() const;
		SharedObject data() const;

		// - modifiers

		void clear() const { BInvoke<void>(NameIDRegistry::Meta::container_clear); }
		template<typename... Args> SharedObject insert(Args&&... args) const;
		template<typename... Args> SharedObject insert_after(Args&&... args) const;
		template<typename... Args> SharedObject insert_or_assign(Args&&... args) const;
		template<typename... Args> SharedObject erase(Args&&... args) const;
		template<typename... Args> SharedObject erase_after(Args&&... args) const;
		template<typename T> void push_front(T&& arg) const { ABInvoke<void>(NameIDRegistry::Meta::container_push_front, std::forward<T>(arg)); };
		template<typename T> void push_back(T&& arg) const { ABInvoke<void>(NameIDRegistry::Meta::container_push_back, std::forward<T>(arg)); };
		void pop_front() const { BInvoke<void>(NameIDRegistry::Meta::container_pop_front); }
		void pop_back() const { BInvoke<void>(NameIDRegistry::Meta::container_pop_back); }
		template<typename T> void push(T&& arg) const { ABInvoke<void>(NameIDRegistry::Meta::container_push, std::forward<T>(arg)); };
		void pop() const { BInvoke<void>(NameIDRegistry::Meta::container_pop); }
		template<typename T> void swap(T&& arg) const { ABInvoke<void>(NameIDRegistry::Meta::container_swap, std::forward<T>(arg)); };
		template<typename T> void merge(T&& arg) const { ABInvoke<void>(NameIDRegistry::Meta::container_merge, std::forward<T>(arg)); };
		template<typename T> SharedObject extract(T&& rhs) const;

		// - list operations

		template<typename... Args> void splice_after(Args&&... args) const { ABInvoke<void>(NameIDRegistry::Meta::container_splice_after, std::forward<Args>(args)...); };
		template<typename... Args> void splice(Args&&... args) const { ABInvoke<void>(NameIDRegistry::Meta::container_splice, std::forward<Args>(args)...); };
		template<typename T> std::size_t remove(T&& arg) const { ABInvoke<std::size_t>(NameIDRegistry::Meta::container_remove, std::forward<T>(arg)); };
		void reverse() const { BInvoke<void>(NameIDRegistry::Meta::container_reverse); }
		template<typename T> std::size_t unique(T&& arg) const { ABInvoke<std::size_t>(NameIDRegistry::Meta::container_unique, std::forward<T>(arg)); };
		void sort() const { BInvoke<void>(NameIDRegistry::Meta::container_sort); }

		// - lookup
		template<typename T> std::size_t count(T&& arg) const { return ABInvoke<std::size_t>(NameIDRegistry::Meta::container_count, std::forward<T>(arg)); };
		template<typename T> SharedObject find(T&& rhs) const;
		template<typename T> SharedObject lower_bound(T&& rhs) const;
		template<typename T> SharedObject upper_bound(T&& rhs) const;
		template<typename T> SharedObject equal_range(T&& rhs) const;

	protected:
		Type type;
		void* ptr; // if type is reference, ptr is a pointer of referenced object
	};
	
	class SharedObject : public ObjectView {
	public:
		using ObjectView::ObjectView;
		using ObjectView::operator=;
	
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

	template<typename T>
	constexpr ObjectView ObjectView_of = { Type_of<T> };
}

#undef OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR

#include "details/Object.inl"
