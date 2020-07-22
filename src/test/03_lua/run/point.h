#include <UDRefl/UDRefl.h>

//
//struct [[info("hello world")]] Point {
//  Point() : x{0.f}, y{0.f} {}
//  Point(float x, float y) : x{0.f}, y{0.f} {}
//	[[not_serialize]]
//	float x;
//	[[range(std::pair<float, float>{0.f, 10.f})]]
//	float y;
//  static size_t num{0};
//
//  float Sum() {
//    return x + y;
//  }
//};
//

namespace Ubpa {
	void Point_init();
	void Point_test();
}
