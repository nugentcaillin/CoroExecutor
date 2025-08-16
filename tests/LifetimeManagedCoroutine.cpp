#include <gtest/gtest.h>
#include <CoroExecutor/CoroExecutor.hpp>
#include <iostream>

class BasicLifetimeManagedCoroutineTest : public testing::Test {
public:

    CoroExecutor::LifetimeManagedCoroutine add_one(int& num)
    {
        num++;
        co_return;
    }
    void resume_helper(CoroExecutor::LifetimeManagedCoroutine& coro)
    {
        coro.resume();
    }
    void destroy_helper(CoroExecutor::LifetimeManagedCoroutine& coro)
    {
        coro.destroy_self();
    }
};

TEST_F(BasicLifetimeManagedCoroutineTest, waitsForResume)
{
    int foo { 1 };
    auto coro = add_one(foo);

    EXPECT_EQ(foo, 1);
    destroy_helper(coro);
}


TEST_F(BasicLifetimeManagedCoroutineTest, executeAfterResume)
{
    int foo { 1 };
    auto coro = add_one(foo);
    resume_helper(coro);

    EXPECT_EQ(foo, 2);
}



TEST(FooTest, bar)
{
    EXPECT_EQ(1, 1);
}