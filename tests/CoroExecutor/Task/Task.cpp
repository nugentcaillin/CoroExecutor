#include <CoroExecutor/CoroExecutor.hpp>
#include <gtest/gtest.h>
#include "Helpers.hpp"
#include "Fixtures.hpp"


TEST_F(SmallTaskDestructionTest, FrameNotDestroyedIfReferenceExists)
{
    ASSERT_EQ(frames_destroyed_.at(0), false);
}

TEST_F(SmallTaskDestructionTest, FrameDestroyedIfLastReference)
{
    tasks_.at(0).~Task();
    ASSERT_EQ(frames_destroyed_.at(0), true);
}

TEST_F(SmallTaskDestructionTest, FrameNotDestroyedWhenCopyAssignedTaskFallsOutOfScope)
{
    auto copy = tasks_.at(0);
    copy.~Task();
    ASSERT_EQ(frames_destroyed_.at(0), false);
}

TEST_F(SmallTaskDestructionTest, FrameDestroyedWhenAllReferencesFallOutOfScopeAfterCopyAssignment)
{
    auto copy = tasks_.at(0);
    copy.~Task();
    tasks_.at(0).~Task();
    ASSERT_EQ(frames_destroyed_.at(0), true);
}

TEST_F(SmallTaskDestructionTest, SelfCopyDoesNotDestroyFrame)
{
    tasks_.at(0) = tasks_.at(0);
    ASSERT_EQ(frames_destroyed_.at(0), false);
}

TEST_F(SmallTaskDestructionTest, FrameDestroyedAfterSelfCopyAndFallingOutOfScope)
{
    tasks_.at(0) = tasks_.at(0);
    tasks_.at(0).~Task();
    ASSERT_EQ(frames_destroyed_.at(0), true);
}


TEST_F(SmallTaskDestructionTest, CopiedIntoTaskDestroysFrameIfNonEmptyAndLastReference)
{
    tasks_.at(0) = tasks_.at(1);
    ASSERT_EQ(frames_destroyed_.at(0), true);
}


TEST_F(SmallTaskDestructionTest, FrameNotDestroyedWhenCopyConstructedTaskFallsOutOfScope)
{
    auto copy(tasks_.at(0));
    copy.~Task();
    ASSERT_EQ(frames_destroyed_.at(0), false);
}

TEST_F(SmallTaskDestructionTest, FrameDestroyedWhenAllReferencesFallOutOfScopeAfterCopyConstructor)
{
    auto copy(tasks_.at(0));
    copy.~Task();
    tasks_.at(0).~Task();
    ASSERT_EQ(frames_destroyed_.at(0), true);
}

TEST_F(SmallTaskDestructionTest, FrameNotDestroyedIfMoveAssignedIntoOtherReference)
{
    auto copy = std::move(tasks_.at(0));
    tasks_.at(0).~Task();
    ASSERT_EQ(frames_destroyed_.at(0), false);
}

TEST_F(SmallTaskDestructionTest, FrameDestroyedIfMoveAssignedReferenceFallsOutOfScopeAndIsLastReference)
{
    auto copy = std::move(tasks_.at(0));
    tasks_.at(0).~Task();
    copy.~Task();
    ASSERT_EQ(frames_destroyed_.at(0), true);

}

TEST_F(SmallTaskDestructionTest, MoveAssignedIntoTaskDestroysFrameIfNonEmptyAndLastReference)
{
    tasks_.at(0) = std::move(tasks_.at(1));
    ASSERT_EQ(frames_destroyed_.at(0), true);
}

TEST_F(SmallTaskDestructionTest, SelfMoveDoesNotDestroyFrame)
{
    tasks_.at(0) = std::move(tasks_.at(0));
    ASSERT_EQ(frames_destroyed_.at(0), false);
}

TEST_F(SmallTaskDestructionTest, FrameDestroyedAfterSelfMoveAndFallingOutOfScope)
{
    tasks_.at(0) = std::move(tasks_.at(0));
    tasks_.at(0).~Task();
    ASSERT_EQ(frames_destroyed_.at(0), true);
}

TEST_F(SmallTaskDestructionTest, FrameNotDestroyedIfMoveConstructedIntoOtherReference)
{
    auto copy(std::move(tasks_.at(0)));
    tasks_.at(0).~Task();
    ASSERT_EQ(frames_destroyed_.at(0), false);
}

TEST_F(SmallTaskDestructionTest, FrameDestroyedIfMoveConstructedReferenceFallsOutOfScopeAndIsLastReference)
{
    auto copy(std::move(tasks_.at(0)));
    tasks_.at(0).~Task();
    copy.~Task();
    ASSERT_EQ(frames_destroyed_.at(0), true);

}

TEST_F(SmallTaskDestructionTest, SwapDoesntDestroyFrame)
{
    std::swap(tasks_.at(0), tasks_.at(1));
    EXPECT_EQ(frames_destroyed_.at(0), false);
    EXPECT_EQ(frames_destroyed_.at(1), false);
}

TEST_F(SmallTaskDestructionTest, SwapCausesCorrectReferenceToDestroyCorrectFrameWhenFallingOutOfScope)
{
    std::swap(tasks_.at(0), tasks_.at(1));
    tasks_.at(0).~Task();
    EXPECT_EQ(frames_destroyed_.at(0), false);
    EXPECT_EQ(frames_destroyed_.at(1), true);
    tasks_.at(1).~Task();
    EXPECT_EQ(frames_destroyed_.at(0), true);
}

TEST_F(MultiThreadedCopyStressTest, FrameNotDestroyedIfReferenceRemains)
{
    joinAll();
    EXPECT_EQ(frames_destroyed_.front(), false);
}

TEST_F(MultiThreadedCopyStressTest, FrameDestroyedWhenLastReferenceDestroyed)
{
    joinAll();
    tasks_.front().~Task();
    EXPECT_EQ(frames_destroyed_.front(), true);
}


TEST_F(SingleCoAwaitTest, CoAwaitingForValueSuspendsExecution)
{
    ASSERT_EQ(count_ready(), 0);
}

TEST_F(SingleCoAwaitTest, AwaiterResumesAwaitingCoroutineOnCompletion)
{
    coro_awaited_many_times_.handle_.resume();
    ASSERT_EQ(count_received_correct_value(), 1);
}


TEST_F(MultiCoAwaitTestSingleThreaded, AllAwaitersSuspend)
{
    ASSERT_EQ(count_ready(), 0); 
}


TEST_F(MultiCoAwaitTestSingleThreaded, AllAwaitersGetCorrectValue)
{
    coro_awaited_many_times_.handle_.resume();
    ASSERT_EQ(count_received_correct_value(), num_to_await_);
}


TEST_F(MultiCoAwaitTestMultiThreaded, AllAwaitersSuspend)
{
    ASSERT_EQ(count_ready(), 0); 
}


TEST_F(MultiCoAwaitTestMultiThreaded, AllAwaitersGetCorrectValue)
{
    coro_awaited_many_times_.handle_.resume();
    ASSERT_EQ(count_received_correct_value(), num_to_await_);
}
