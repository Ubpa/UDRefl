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
	void f(ObjectView obj) {
		std::cout << "f(ObjectView) [" << obj.GetType().GetName() << "]" << obj << std::endl;
	}
	void g(const char*) {
		std::cout << "g(const char*)" << std::endl;
	}
	void g(int(&&)[]) {
		std::cout << "g(int(&&)[])" << std::endl;
	}
};

int main() {
	Mngr->RegisterType<Funcs>();
	Mngr->AddMethod<MemFuncOf<Funcs, void(double)>::get(&Funcs::f)>("f");
	Mngr->AddMethod<MemFuncOf<Funcs, void(std::uint8_t&)>::get(&Funcs::f)>("f");
	Mngr->AddMethod<MemFuncOf<Funcs, void(const std::uint8_t&)>::get(&Funcs::f)>("f");
	Mngr->AddMethod<MemFuncOf<Funcs, void(std::uint8_t&&)>::get(&Funcs::f)>("f");
	Mngr->AddMethod<MemFuncOf<Funcs, void(const std::uint8_t&&)>::get(&Funcs::f)>("f");
	Mngr->AddMethod<MemFuncOf<Funcs, void(ObjectView)>::get(&Funcs::f)>("f");
	Mngr->AddMethod<MemFuncOf<Funcs, void(const char*)>::get(&Funcs::g)>("g");
	Mngr->AddMethod<MemFuncOf<Funcs, void(int(&&)[])>::get(&Funcs::g)>("g");

	SharedObject funcs = Mngr->MakeShared(Type_of<Funcs>);

	std::uint8_t i = 1;
	const std::uint8_t ci = 1;
	funcs.BInvoke<void>("f", MethodFlag::All, 1.);
	funcs.BInvoke<void>("f", MethodFlag::All, 1.f);
	funcs.BInvoke<void>("f", MethodFlag::All, 1);
	funcs.BInvoke<void>("f", MethodFlag::All, i);
	funcs.BInvoke<void>("f", MethodFlag::All, ci);
	funcs.BInvoke<void>("f", MethodFlag::All, std::move(i));
	funcs.BInvoke<void>("f", MethodFlag::All, static_cast<const std::uint8_t&&>(ci));
	funcs.BInvoke<void>("f", MethodFlag::All, std::string_view{ "hello world" });
	funcs.BInvoke<void>("f", MethodFlag::All, std::string{ "hello world" });

	int arr_i[5];
	funcs.BInvoke<void>("g", MethodFlag::All, "hello"); // const char(&)[6]
	funcs.BInvoke<void>("g", MethodFlag::All, arr_i);   // int(&)[5]
}
