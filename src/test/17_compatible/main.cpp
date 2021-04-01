#include <UDRefl/UDRefl.hpp>
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
	void f(ObjectView obj) {
		std::cout << "f(ObjectView) [" << obj.GetType().GetName() << "]" << obj << std::endl;
	}
	void g(const char*) {
		std::cout << "g(const char*)" << std::endl;
	}
	void g(const int(&&)[]) {
		std::cout << "g(const int(&&)[])" << std::endl;
	}
};

int main() {
	Mngr.RegisterType<Funcs>();
	Mngr.AddMethod<MemFuncOf<Funcs, void(double)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<Funcs, void(std::uint8_t&)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<Funcs, void(const std::uint8_t&)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<Funcs, void(std::uint8_t&&)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<Funcs, void(const std::uint8_t&&)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<Funcs, void(ObjectView)>::get(&Funcs::f)>("f");
	Mngr.AddMethod<MemFuncOf<Funcs, void(const char*)>::get(&Funcs::g)>("g");
	Mngr.AddMethod<MemFuncOf<Funcs, void(const int(&&)[])>::get(&Funcs::g)>("g");

	SharedObject funcs = Mngr.MakeShared(Type_of<Funcs>);

	std::uint8_t i = 1;
	const std::uint8_t ci = 1;
	funcs.Invoke<void>("f", TempArgsView{ 1. });
	funcs.Invoke<void>("f", TempArgsView{ 1.f });
	funcs.Invoke<void>("f", TempArgsView{ 1 });
	funcs.Invoke<void>("f", TempArgsView{ i });
	funcs.Invoke<void>("f", TempArgsView{ ci });
	funcs.Invoke<void>("f", TempArgsView{ std::move(i) });
	funcs.Invoke<void>("f", TempArgsView{ static_cast<const std::uint8_t&&>(ci) });
	funcs.Invoke<void>("f", TempArgsView{ std::string_view{ "hello world" } });
	funcs.Invoke<void>("f", TempArgsView{ std::string{ "hello world" } });

	int arr_i[5];
	funcs.Invoke<void>("g", TempArgsView{ "hello" }); // const char(&)[6]
	funcs.Invoke<void>("g", TempArgsView{ arr_i }); // int(&)[5]
}
