#include <gtest/gtest.h>
#include "../src/Task.h"

TEST(TaskTest, MarksDoneCorrectly) {
    Task t("Read a book", false);
    t.markDone();
    EXPECT_TRUE(t.isDone());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}