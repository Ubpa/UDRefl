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
};

int main() {
	Mngr.RegisterType<Funcs>();
	Mngr.AddMethod<MemFuncOf<void(double)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<void(std::uint8_t&)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<void(const std::uint8_t&)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<void(std::uint8_t&&)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<void(const std::uint8_t&&)>::get(&Funcs::f)>("f");

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
}
