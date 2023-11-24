#include "gtest/gtest.h"
#include "Maths.h"


namespace dae
{
	TEST(TestCaseName, TestName) {
		EXPECT_EQ(Vector3::Cross(Vector3::UnitX, Vector3::UnitY), Vector3::UnitZ);
		EXPECT_TRUE(true);
	}

}
