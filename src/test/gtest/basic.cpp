#include <gtest/gtest.h>

#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Point { float x, y; };

class PointTest : public testing::Test {
public:
	void SetUp() override {
		Mngr.RegisterType<Point>();
		Mngr.AddField<&Point::x>("x");
		Mngr.AddField<&Point::y>("y");
	}
	virtual void TearDown() {
		Mngr.typeinfos.erase(Type_of<Point>);
	}
};

TEST_F(PointTest, Ctor) {
	SharedObject p = Mngr.MakeShared(Type_of<Point>);
	EXPECT_TRUE(p.GetType().Valid());
	EXPECT_EQ(p.GetType(), Type_of<Point>);
	EXPECT_NE(p.GetPtr(), nullptr);
}

TEST_F(PointTest, Var) {
	Point p;
	ObjectView obj{ p };
	obj.Var("x") = 1.f;
	obj.Var("y") = 2.f;
	EXPECT_EQ(obj.Var("x"), 1.f);
	EXPECT_EQ(obj.Var("y"), 2.f);
}

TEST_F(PointTest, GetVars) {
	Point p{ 1.f,2.f };
	ObjectView obj{ p };
	auto vars = obj.GetVars();
	auto iter_begin = vars.begin();
	auto iter_end = vars.end();

	auto cursor = iter_begin;

	{
		EXPECT_NE(cursor, iter_end);
		const auto& [name, var] = *cursor;
		if (name == "x")
			EXPECT_EQ(var, 1.f);
		else if (name == "y")
			EXPECT_EQ(var, 2.f);
		else
			EXPECT_TRUE(false);
	}
	++cursor;
	{
		EXPECT_NE(cursor, iter_end);
		const auto& [name, var] = *cursor;
		if (name == "x")
			EXPECT_EQ(var, 1.f);
		else if (name == "y")
			EXPECT_EQ(var, 2.f);
		else
			EXPECT_TRUE(false);
	}
	++cursor;
	EXPECT_EQ(cursor, iter_end);
}
