#include <UDRefl/UDRefl.h>
#include <iostream>
#include <cmath>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Funcs {
	void f(double) {
		std::cout << "f(double)" << std::endl;
	}
	void f(std::uint8_t&) {
		std::cout << "f(std::uint8_t&)" << std::endl;
	}
	void f(const std::uint8_t&) {
		std::cout << "f(const std::uint8_t&)" << std::endl;
	}
	void f(std::uint8_t&&) {
		std::cout << "f(std::uint8_t&&)" << std::endl;
	}
	void f(const std::uint8_t&&) {
		std::cout << "f(const std::uint8_t&&)" << std::endl;
	}
	void g(const char*) {
		std::cout << "f(const char*)" << std::endl;
	}
	void g(int(&&)[]) {
		std::cout << "f(int(&&)[])" << std::endl;
	}
};

int main() {
	Mngr.RegisterType<Funcs>();
	Mngr.AddMethod<MemFuncOf<Funcs, void(double)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<Funcs, void(std::uint8_t&)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<Funcs, void(const std::uint8_t&)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<Funcs, void(std::uint8_t&&)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<Funcs, void(const std::uint8_t&&)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<Funcs, void(const char*)>::get(&Funcs::g)>("g");
	Mngr.AddMethod<MemFuncOf<Funcs, void(int(&&)[])>::get(&Funcs::g)>("g");

	SharedObject funcs = Mngr.MakeShared(Type_of<Funcs>);

	std::uint8_t i = 1;
	const std::uint8_t ci = 1;
	funcs.Invoke<void>("f", 1.);
	funcs.Invoke<void>("f", 1.f);
	funcs.Invoke<void>("f", 1);
	funcs.Invoke<void>("f", i);
	funcs.Invoke<void>("f", ci);
	funcs.Invoke<void>("f", std::move(i));
	funcs.Invoke<void>("f", static_cast<const std::uint8_t&&>(ci));

	int arr_i[5];
	funcs.Invoke<void>("g", "hello"); // const char(&)[6]
	funcs.Invoke<void>("g", arr_i);   // int(&)[5]
}
