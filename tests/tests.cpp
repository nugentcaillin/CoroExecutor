#include <gtest/gtest.h>
#include <CoroExecutor/test.hpp>

TEST(AdditionTest, HandlesPositive)
{
  EXPECT_EQ(test::add(1, 2), 3);
}


TEST(AdditionTest, HandlesNegative)
{
  EXPECT_EQ(test::add(-1, -2), -3);
}

TEST(AdditionTest, HandlesPositiveAndNegative)
{
  EXPECT_EQ(test::add(1, -2), -1);
  EXPECT_EQ(test::add(-1, 2), 1);
}

TEST(SubtractionTest, HandlesPositive)
{
  EXPECT_EQ(test::subtract(1, 2), -1);
}


TEST(SubtractionTest, HandlesNegative)
{
  EXPECT_EQ(test::subtract(-1, -2), 1);
}

TEST(SubtractionTest, HandlesPositiveAndNegative)
{
  EXPECT_EQ(test::subtract(1, -2), 3);
  EXPECT_EQ(test::subtract(-1, 2), -3);
}
