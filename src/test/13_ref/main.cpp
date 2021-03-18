#include <UDRefl/UDRefl.hpp>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Data {
	Data(float value) : value{ value } {}
	Data(const Data& f) :value{ f.value } {
		std::cout << "copy ctor" << std::endl;
	}
	Data(Data&& f) noexcept :value{ f.value } {
		std::cout << "move ctor" << std::endl;
	}
	Data& operator=(const Data& rhs) {
		value = rhs.value;
		return *this;
	}
	Data& operator=(Data&& rhs) noexcept {
		value = rhs.value;
		return *this;
	}
	float value;
};

struct A {
	A(Data& lref, Data&& rref) :lref{ lref }, rref{ std::move(rref) }{}
	Data& lref;
	Data&& rref;
	Data get() {
		return lref;
	}
	const Data& get_c() const {
		return lref;
	}
	Data& get_l() {
		return lref;
	}
	Data&& get_r() {
		return std::move(lref);
	}
	void set(Data d) {
		lref = d;
	}
	void set_c(const Data& d) {
		lref = d;
	}
	void set_l(Data& d) {
		lref = d;
	}
	void set_r(Data&& d) {
		lref = std::move(d);
	}
};

int main() {
	Mngr.RegisterType<A>();
	Mngr.RegisterType<Data>();
	Mngr.AddField<&Data::value>("value");
	Mngr.AddConstructor<A, Data&, Data&&>();
	Mngr.AddField("lref", [](A* a) -> decltype(auto) {
		return a->lref;
	});
	Mngr.AddField("rref", [](A* a) -> decltype(auto) {
		return &a->rref;
	});
	Mngr.AddMethod<&A::get>("get");
	Mngr.AddMethod<&A::get_c>("get_c");
	Mngr.AddMethod<&A::get_l>("get_l");
	Mngr.AddMethod<&A::get_r>("get_r");
	Mngr.AddMethod<&A::set>("set");
	Mngr.AddMethod<&A::set_c>("set_c");
	Mngr.AddMethod<&A::set_l>("set_l");
	Mngr.AddMethod<&A::set_r>("set_r");

	Data f = 1.f;
	Data g = 2.f;
	auto a = Mngr.MakeShared(Type_of<A>, TempArgsView{ f, std::move(g) });
	std::cout << "a.rref: " << a.Var("rref").Var("value") << std::endl;
	std::cout << "a.lref: " << a.Var("lref").Var("value") << std::endl;

	a.Var("lref").Var("value") = 2.f;
	a.Var("rref").Var("value") = 3.f;
	std::cout << "a.lref: " << a.Var("lref").Var("value") << std::endl;
	std::cout << "a.rref: " << a.Var("rref").Var("value") << std::endl;
	std::cout << "f: " << f.value << std::endl;
	std::cout << "g: " << g.value << std::endl;

	std::cout << "a.get_r(): " << a.Invoke("get_r").Var("value") << std::endl;
	std::cout << "a.get_c(): " << a.Invoke("get_r").Var("value") << std::endl;

	a.Invoke<void>("set", TempArgsView{ g });
	a.Invoke<void>("set_r", TempArgsView{ std::move(g) });
	std::cout << "a.lref: " << a.Invoke("get_r").Var("value") << std::endl;
	std::cout << "a.get_c(): " << a.Invoke("get_c").Var("value") << std::endl;
	return 0;
}
