#pragma once

#define OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(op, name)                                                              \
template<typename T>                                                                                                 \
ObjectView ObjectView::operator op (T&& rhs) const {                                                                 \
    Invoke<void>(NameIDRegistry::Meta::operator_##name, TempArgsView{ std::forward<T>(rhs) }, MethodFlag::Variable); \
    return AddLValueReference();                                                                                     \
}

#define OBJECT_VIEW_DEFINE_META_T(fname, mname)                                       \
template<typename T>                                                                  \
SharedObject ObjectView::fname (T&& arg) const {                                      \
    return Invoke(NameIDRegistry::Meta::mname, TempArgsView{ std::forward<T>(arg) }); \
}

#define OBJECT_VIEW_DEFINE_OPERATOR_T(op, name) OBJECT_VIEW_DEFINE_META_T(operator op, operator_##name)

#define OBJECT_VIEW_DEFINE_CONTAINER_META_T(name) OBJECT_VIEW_DEFINE_META_T(name, container_##name)

#define OBJECT_VIEW_DEFINE_META_VARS_T(prefix, name)                                                   \
template<typename... Args>                                                                             \
SharedObject ObjectView::name (Args&&... args) const {                                                 \
    return Invoke(NameIDRegistry::Meta::prefix##_##name, TempArgsView{ std::forward<Args>(args)... }); \
}

#define DEFINE_OPERATOR_LSHIFT(Lhs, Rhs)            \
inline Lhs& operator <<(Lhs& lhs, const Rhs& rhs) { \
    rhs >> lhs;                                     \
    return lhs;                                     \
}

#define DEFINE_OPERATOR_RSHIFT(Lhs, Rhs)            \
inline Lhs& operator >>(Lhs& lhs, const Rhs& rhs) { \
	rhs << lhs;                                     \
	return lhs;                                     \
}

namespace Ubpa::UDRefl::details {
	template<typename T>
	constexpr Type ArgType(const std::remove_const_t<std::remove_reference_t<T>>& arg) noexcept {
		using U = std::remove_cvref_t<T>;
		if constexpr (std::is_same_v<U, ObjectView> || std::is_same_v<U, SharedObject>)
			return ObjectView{ arg }.AddLValueReferenceWeak().GetType();
		else
			return Type_of<T>;
	}

	template<typename T>
	constexpr void* ArgPtr(const T& arg) noexcept {
		if constexpr (std::is_same_v<T, ObjectView> || std::is_same_v<T, SharedObject>)
			return arg.GetPtr();
		else
			return const_cast<void*>(static_cast<const void*>(&arg));
	}
}

namespace Ubpa::UDRefl {
	constexpr ObjectView ArgsView::operator[](size_t idx) const noexcept
	{ return { argTypes[idx], buffer[idx] }; }

	template<std::size_t N>
	template<typename... Args>
	TempArgsView<N>::TempArgsView(Args&&... args) noexcept :
		argTypes{ details::ArgType<decltype(args)>(args)... },
		argptr_buffer{ details::ArgPtr(args)... }
	{ static_assert(sizeof...(Args) == N); }

	template<typename T>
	constexpr auto* ObjectView::AsPtr() const noexcept {
		assert(type.Is<T>());
		return reinterpret_cast<std::add_pointer_t<T>>(ptr);
	}

	template<typename T>
	constexpr decltype(auto) ObjectView::As() const noexcept {
		assert(ptr);
		auto* ptr = AsPtr<T>();
		if constexpr (std::is_reference_v<T>)
			return std::forward<T>(*ptr);
		else
			return *ptr;
	}

	template<typename T>
	auto SharedObject::AsShared() const {
		static_assert(!std::is_reference_v<T>);
		assert(!IsObjectView());
		assert(GetType().template Is<T>());
		return std::reinterpret_pointer_cast<T>(buffer);
	}

	//////////////
	// ReflMngr //
	//////////////

	constexpr ObjectView ObjectView::RemoveConst() const noexcept { return { type.RemoveConst(), ptr }; }
	constexpr ObjectView ObjectView::RemoveLValueReference() const noexcept { return { type.RemoveLValueReference(), ptr }; }
	constexpr ObjectView ObjectView::RemoveRValueReference() const noexcept { return { type.RemoveRValueReference(), ptr }; }
	constexpr ObjectView ObjectView::RemoveReference() const noexcept { return { type.RemoveReference(), ptr }; }
	constexpr ObjectView ObjectView::RemoveConstReference() const noexcept { return { type.RemoveCVRef(), ptr }; }

	template<typename... Args>
	Type ObjectView::IsInvocable(Name method_name, MethodFlag flag) const {
		if constexpr (sizeof...(Args) > 0) {
			constexpr Type argTypes[] = { Type_of<Args>... };
			return IsInvocable(method_name, argTypes, flag);
		}
		else
			return IsInvocable(method_name, {}, flag);
	}

	template<typename T>
	T ObjectView::Invoke(
		Name method_name,
		ArgsView args,
		MethodFlag flag,
		std::pmr::memory_resource* temp_args_rsrc) const
	{
		if constexpr (!std::is_void_v<T>) {
			using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
			std::aligned_storage_t<sizeof(U), alignof(U)> result_buffer;
			Type result_type = BInvoke(method_name, &result_buffer, args, flag, temp_args_rsrc);
			return MoveResult<T>(result_type, &result_buffer);
		}
		else
			BInvoke(method_name, nullptr, args, flag, temp_args_rsrc);
	}
	
	//////////
	// Meta //
	//////////

	//
	// operators
	//////////////

	inline SharedObject ObjectView::operator++() const { return Invoke(NameIDRegistry::Meta::operator_pre_inc); }
	inline SharedObject ObjectView::operator++(int) const { return Invoke(NameIDRegistry::Meta::operator_post_inc); }
	inline SharedObject ObjectView::operator--() const { return Invoke(NameIDRegistry::Meta::operator_pre_dec); }
	inline SharedObject ObjectView::operator--(int) const { return Invoke(NameIDRegistry::Meta::operator_post_dec); }
	inline SharedObject ObjectView::operator+() const { return Invoke(NameIDRegistry::Meta::operator_add); }
	inline SharedObject ObjectView::operator-() const { return Invoke(NameIDRegistry::Meta::operator_sub); }
	inline SharedObject ObjectView::operator~() const { return Invoke(NameIDRegistry::Meta::operator_bnot); }
	inline SharedObject ObjectView::operator*() const { return Invoke(NameIDRegistry::Meta::operator_indirection); }

	OBJECT_VIEW_DEFINE_OPERATOR_T(+, add)
	OBJECT_VIEW_DEFINE_OPERATOR_T(-, sub)
	OBJECT_VIEW_DEFINE_OPERATOR_T(*, mul)
	OBJECT_VIEW_DEFINE_OPERATOR_T(/, div)
	OBJECT_VIEW_DEFINE_OPERATOR_T(%, mod)
	OBJECT_VIEW_DEFINE_OPERATOR_T(&, band)
	OBJECT_VIEW_DEFINE_OPERATOR_T(|, bor)
	OBJECT_VIEW_DEFINE_OPERATOR_T(^, bxor)
	OBJECT_VIEW_DEFINE_OPERATOR_T(<<, shl)
	OBJECT_VIEW_DEFINE_OPERATOR_T(>>, shr)

	OBJECT_VIEW_DEFINE_OPERATOR_T([], subscript)

	template<typename T> bool ObjectView::operator< (const T& rhs) const
	{ return Invoke<bool>(NameIDRegistry::Meta::operator_lt, TempArgsView{ rhs }, MethodFlag::Const); }
	template<typename T> bool ObjectView::operator<=(const T& rhs) const
	{ return Invoke<bool>(NameIDRegistry::Meta::operator_le, TempArgsView{ rhs }, MethodFlag::Const); }
	template<typename T> bool ObjectView::operator> (const T& rhs) const
	{ return Invoke<bool>(NameIDRegistry::Meta::operator_gt, TempArgsView{ rhs }, MethodFlag::Const); }
	template<typename T> bool ObjectView::operator>=(const T& rhs) const
	{ return Invoke<bool>(NameIDRegistry::Meta::operator_ge, TempArgsView{ rhs }, MethodFlag::Const); }

	template<typename T> requires NonObjectAndView<std::decay_t<T>>
	ObjectView ObjectView::operator=(T&& rhs) const {
		Invoke<void>(NameIDRegistry::Meta::operator_assignment, TempArgsView{ std::forward<T>(rhs) }, MethodFlag::Variable);
		return AddLValueReference();
	}

	OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(+=, assignment_add);
	OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(-=, assignment_sub);
	OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(*=, assignment_mul);
	OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(/=, assignment_div);
	OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(%=, assignment_mod);
	OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(&=, assignment_band);
	OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(|=, assignment_bor);
	OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(^=, assignment_bxor);
	OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(<<=, assignment_shl);
	OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(>>=, assignment_shr);

	template<typename... Args>
	SharedObject ObjectView::operator()(Args&&... args) const
	{ return AInvoke(NameIDRegistry::Meta::operator_call, TempArgsView{ std::forward<Args>(args)... }); }

	//
	// non-member functions
	/////////////////////////

	inline ObjectView ObjectView::get(std::size_t i) const
	{ return Invoke<ObjectView>(NameIDRegistry::Meta::get, TempArgsView{ std::move(i) }, MethodFlag::Member); }
	inline ObjectView ObjectView::get(Type type) const
	{ return Invoke<ObjectView>(NameIDRegistry::Meta::get, TempArgsView{ std::move(type) }, MethodFlag::Member); }
	inline ObjectView ObjectView::variant_visit_get() const
	{ return Invoke<ObjectView>(NameIDRegistry::Meta::variant_visit_get, ArgsView{}, MethodFlag::Member); }

	inline std::size_t ObjectView::tuple_size() const
	{ return Invoke<std::size_t>(NameIDRegistry::Meta::tuple_size, ArgsView{}, MethodFlag::Static); }
	inline Type ObjectView::tuple_element(std::size_t i) const
	{ return Invoke<Type>(NameIDRegistry::Meta::tuple_element, TempArgsView{ std::move(i) }, MethodFlag::Static); }

	inline std::size_t ObjectView::variant_size() const
	{ return Invoke<std::size_t>(NameIDRegistry::Meta::variant_size, ArgsView{}, MethodFlag::Static); }
	inline Type ObjectView::variant_alternative(std::size_t i) const
	{ return Invoke<Type>(NameIDRegistry::Meta::variant_alternative, TempArgsView{ std::move(i) }, MethodFlag::Static); }

	template<typename T> void ObjectView::advance(T&& arg) const
	{ Invoke<void>(NameIDRegistry::Meta::advance, TempArgsView{ std::forward<T>(arg) }, MethodFlag::Variable); };
	template<typename T> std::size_t ObjectView::distance(T&& arg) const
	{ return Invoke<std::size_t>(NameIDRegistry::Meta::distance, TempArgsView{ std::forward<T>(arg) }, MethodFlag::Const); };
	OBJECT_VIEW_DEFINE_META_T(next, next);
	OBJECT_VIEW_DEFINE_META_T(prev, prev);
	inline SharedObject ObjectView::next() const { return Invoke(NameIDRegistry::Meta::next); }
	inline SharedObject ObjectView::prev() const { return Invoke(NameIDRegistry::Meta::prev); }

	//
	// member functions
	/////////////////////

	template<typename... Args> void ObjectView::assign(Args&&... args) const
	{ Invoke<void>(NameIDRegistry::Meta::container_assign, TempArgsView{ std::forward<Args>(args)... }, MethodFlag::Variable); };

	// - element access

	OBJECT_VIEW_DEFINE_CONTAINER_META_T(at)
	inline SharedObject ObjectView::data() const { return Invoke(NameIDRegistry::Meta::container_data); }
	inline SharedObject ObjectView::front() const { return Invoke(NameIDRegistry::Meta::container_front); }
	inline SharedObject ObjectView::back() const { return Invoke(NameIDRegistry::Meta::container_back); }
	inline SharedObject ObjectView::top() const { return Invoke(NameIDRegistry::Meta::container_top); }

	// - iterator

	inline SharedObject ObjectView::begin() const { return Invoke(NameIDRegistry::Meta::container_begin); }
	inline SharedObject ObjectView::end() const { return Invoke(NameIDRegistry::Meta::container_end); }
	inline SharedObject ObjectView::rbegin() const { return Invoke(NameIDRegistry::Meta::container_rbegin); }
	inline SharedObject ObjectView::rend() const { return Invoke(NameIDRegistry::Meta::container_rend); }
	inline SharedObject ObjectView::cbegin() const { return Invoke(NameIDRegistry::Meta::container_cbegin); }
	inline SharedObject ObjectView::cend() const { return Invoke(NameIDRegistry::Meta::container_cend); }
	inline SharedObject ObjectView::crbegin() const { return Invoke(NameIDRegistry::Meta::container_crbegin); }
	inline SharedObject ObjectView::crend() const { return Invoke(NameIDRegistry::Meta::container_crend); }

	// - capacity

	inline bool ObjectView::empty() const
	{ return Invoke<bool>(NameIDRegistry::Meta::container_empty, ArgsView{}, MethodFlag::Const); }
	inline std::size_t ObjectView::size() const
	{ return Invoke<std::size_t>(NameIDRegistry::Meta::container_size, ArgsView{}, MethodFlag::Const); }
	inline std::size_t ObjectView::capacity() const
	{ return Invoke<std::size_t>(NameIDRegistry::Meta::container_capacity, ArgsView{}, MethodFlag::Const); }
	inline std::size_t ObjectView::bucket_count() const
	{ return Invoke<std::size_t>(NameIDRegistry::Meta::container_bucket_count, ArgsView{}, MethodFlag::Const); }
	template<typename... Args> void ObjectView::resize(Args&&... args) const
	{ Invoke<void>(NameIDRegistry::Meta::container_resize, TempArgsView{ std::forward<Args>(args)... }, MethodFlag::Variable); };
	inline void ObjectView::reserve(std::size_t n) const
	{ Invoke<void>(NameIDRegistry::Meta::container_reserve, TempArgsView{ std::move(n) }, MethodFlag::Variable); }
	inline void ObjectView::shrink_to_fit() const
	{ Invoke<void>(NameIDRegistry::Meta::container_shrink_to_fit, ArgsView{}, MethodFlag::Variable); }

	// - modifiers

	inline void ObjectView::clear() const
	{ Invoke<void>(NameIDRegistry::Meta::container_clear, ArgsView{}, MethodFlag::Variable); }
	OBJECT_VIEW_DEFINE_META_VARS_T(container, insert)
	OBJECT_VIEW_DEFINE_META_VARS_T(container, insert_after)
	OBJECT_VIEW_DEFINE_META_VARS_T(container, insert_or_assign)
	OBJECT_VIEW_DEFINE_META_VARS_T(container, erase)
	OBJECT_VIEW_DEFINE_META_VARS_T(container, erase_after)
	template<typename T> void ObjectView::push_front(T&& arg) const
	{ Invoke<void>(NameIDRegistry::Meta::container_push_front, TempArgsView{ std::forward<T>(arg) }, MethodFlag::Variable); };
	template<typename T> void ObjectView::push_back(T&& arg) const
	{ Invoke<void>(NameIDRegistry::Meta::container_push_back, TempArgsView{ std::forward<T>(arg) }, MethodFlag::Variable); };
	inline void ObjectView::pop_front() const
	{ Invoke<void>(NameIDRegistry::Meta::container_pop_front, ArgsView{}, MethodFlag::Variable); }
	inline void ObjectView::pop_back() const
	{ Invoke<void>(NameIDRegistry::Meta::container_pop_back, ArgsView{}, MethodFlag::Variable); }
	template<typename T> void ObjectView::push(T&& arg) const
	{ Invoke<void>(NameIDRegistry::Meta::container_push, TempArgsView{ std::forward<T>(arg) }, MethodFlag::Variable); };
	inline void ObjectView::pop() const
	{ Invoke<void>(NameIDRegistry::Meta::container_pop, ArgsView{}, MethodFlag::Variable); }
	template<typename T> void ObjectView::swap(T&& arg) const
	{ Invoke<void>(NameIDRegistry::Meta::container_swap, TempArgsView{ std::forward<T>(arg) }, MethodFlag::Variable); };
	template<typename T> void ObjectView::merge(T&& arg) const
	{ Invoke<void>(NameIDRegistry::Meta::container_merge, TempArgsView{ std::forward<T>(arg) }, MethodFlag::Variable); };
	OBJECT_VIEW_DEFINE_CONTAINER_META_T(extract)

	// - list operations

	template<typename... Args> void ObjectView::splice_after(Args&&... args) const
	{ Invoke<void>(NameIDRegistry::Meta::container_splice_after, TempArgsView{ std::forward<Args>(args)... }, MethodFlag::Variable); };
	template<typename... Args> void ObjectView::splice(Args&&... args) const
	{ Invoke<void>(NameIDRegistry::Meta::container_splice, TempArgsView{ std::forward<Args>(args)... }, MethodFlag::Variable); };
	template<typename T> std::size_t ObjectView::remove(T&& arg) const
	{ return Invoke<std::size_t>(NameIDRegistry::Meta::container_remove, TempArgsView{ std::forward<T>(arg) }, MethodFlag::Variable); };
	inline void ObjectView::reverse() const
	{ Invoke<void>(NameIDRegistry::Meta::container_reverse, ArgsView{}, MethodFlag::Variable); }
	template<typename T> std::size_t ObjectView::unique(T&& arg) const
	{ return Invoke<std::size_t>(NameIDRegistry::Meta::container_unique, TempArgsView{ std::forward<T>(arg) }, MethodFlag::Variable); };
	inline void ObjectView::sort() const
	{ Invoke<void>(NameIDRegistry::Meta::container_sort, ArgsView{}, MethodFlag::Variable); }

	// - lookup
	
	template<typename T> std::size_t ObjectView::count(T&& arg) const
	{ return Invoke<std::size_t>(NameIDRegistry::Meta::container_count, MethodFlag::Const, std::forward<T>(arg)); };
	OBJECT_VIEW_DEFINE_CONTAINER_META_T(find)
	OBJECT_VIEW_DEFINE_CONTAINER_META_T(lower_bound)
	OBJECT_VIEW_DEFINE_CONTAINER_META_T(upper_bound)
	OBJECT_VIEW_DEFINE_CONTAINER_META_T(equal_range)

	// - variant

	inline std::size_t ObjectView::index() const
	{ return Invoke<std::size_t>(NameIDRegistry::Meta::variant_index, ArgsView{}, MethodFlag::Const); }
	inline bool ObjectView::holds_alternative(Type type) const
	{ return Invoke<bool>(NameIDRegistry::Meta::holds_alternative, TempArgsView{ std::move(type) }, MethodFlag::Const); }

	// - optional

	inline bool ObjectView::has_value() const
	{ return Invoke<bool>(NameIDRegistry::Meta::optional_has_value, ArgsView{}, MethodFlag::Const); }
	inline ObjectView ObjectView::value() const
	{ return Invoke<ObjectView>(NameIDRegistry::Meta::optional_value, ArgsView{}, MethodFlag::Member); }
	inline void ObjectView::reset() const
	{ Invoke<void>(NameIDRegistry::Meta::optional_reset, ArgsView{}, MethodFlag::Variable); }
}

template<>
struct std::hash<Ubpa::UDRefl::ObjectView> {
	std::size_t operator()(const Ubpa::UDRefl::ObjectView& obj) const noexcept {
		return obj.GetType().GetID().GetValue() ^ std::hash<const void*>()(obj.GetPtr());
	}
};

template<>
struct std::hash<Ubpa::UDRefl::SharedObject> {
	std::size_t operator()(const Ubpa::UDRefl::SharedObject& obj) const noexcept {
		return obj.GetType().GetID().GetValue() ^ std::hash<const void*>()(obj.GetPtr());
	}
};

namespace std {
	inline void swap(Ubpa::UDRefl::SharedObject& left, Ubpa::UDRefl::SharedObject& right) noexcept {
		left.Swap(right);
	}
}

namespace Ubpa::UDRefl {
	inline bool operator== (const ObjectView& lhs, const ObjectView& rhs) {
		return lhs.Invoke<bool>(NameIDRegistry::Meta::operator_eq, TempArgsView{ rhs }, MethodFlag::Const)
			|| rhs.Invoke<bool>(NameIDRegistry::Meta::operator_eq, TempArgsView{ lhs }, MethodFlag::Const);
	}

	inline bool operator!= (const ObjectView& lhs, const ObjectView& rhs) { return !(lhs == rhs); }

	template<NonObjectAndView T> bool operator==(const T& lhs, ObjectView ptr) { return ObjectView{ lhs } == ptr; }
	template<NonObjectAndView T> bool operator!=(const T& lhs, ObjectView ptr) { return ObjectView{ lhs } != ptr; }
	template<NonObjectAndView T> bool operator< (const T& lhs, ObjectView ptr) { return ObjectView{ lhs } <  ptr; }
	template<NonObjectAndView T> bool operator<=(const T& lhs, ObjectView ptr) { return ObjectView{ lhs } <= ptr; }
	template<NonObjectAndView T> bool operator> (const T& lhs, ObjectView ptr) { return ObjectView{ lhs } >  ptr; }
	template<NonObjectAndView T> bool operator>=(const T& lhs, ObjectView ptr) { return ObjectView{ lhs } >= ptr; }

	DEFINE_OPERATOR_LSHIFT(std::ostream, ObjectView)
	DEFINE_OPERATOR_LSHIFT(std::ostringstream, ObjectView)
	DEFINE_OPERATOR_LSHIFT(std::ofstream, ObjectView)
	DEFINE_OPERATOR_LSHIFT(std::iostream, ObjectView)
	DEFINE_OPERATOR_LSHIFT(std::stringstream, ObjectView)
	DEFINE_OPERATOR_LSHIFT(std::fstream, ObjectView)

	DEFINE_OPERATOR_RSHIFT(std::istream, ObjectView)
	DEFINE_OPERATOR_RSHIFT(std::istringstream, ObjectView)
	DEFINE_OPERATOR_RSHIFT(std::ifstream, ObjectView)
	DEFINE_OPERATOR_RSHIFT(std::iostream, ObjectView)
	DEFINE_OPERATOR_RSHIFT(std::stringstream, ObjectView)
	DEFINE_OPERATOR_RSHIFT(std::fstream, ObjectView)
}

#undef DEFINE_OPERATOR_RSHIFT
#undef DEFINE_OPERATOR_LSHIFT
#undef OBJECT_VIEW_DEFINE_META_VARS_T
#undef OBJECT_VIEW_DEFINE_CONTAINER_META_T
#undef OBJECT_VIEW_DEFINE_OPERATOR_T
#undef OBJECT_VIEW_DEFINE_META_T
#undef OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR
