#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include "../lib/includes/core/results.h"

#include "../lib/includes/core/DaikonDB.h"

using namespace daikon;
using namespace daikon::core;

class MemoryTest : public ::testing::Test {
   protected:
    DaikonDatabase database_{0};
    size_t empty_base_memory_ = 0;

    void SetUp() override {
        empty_base_memory_ = database_.GetTotalRamUsage();
    }

    void TearDown() override {
        database_.Flushdb();
    }
};

TEST_F(MemoryTest, InitialState) {
    EXPECT_EQ(database_.Dbsize().value, 0);
    EXPECT_EQ(database_.ConfigGetMaxmemory().value, 0);
}

TEST_F(MemoryTest, MemoryUsageForKey) {
    size_t before_set = database_.GetTotalRamUsage();

    auto set_res = database_.Set("mykey", "hello world");
    ASSERT_TRUE(set_res.has_value());
    ASSERT_TRUE(set_res->success);

    auto usage = database_.MemoryUsage("mykey");
    EXPECT_GT(usage.value, 0);

    size_t after_set = database_.GetTotalRamUsage();
    EXPECT_GT(after_set, before_set);

    database_.Del({"mykey"});
    database_.Flushdb();
    EXPECT_EQ(database_.GetTotalRamUsage(), empty_base_memory_);
}

TEST_F(MemoryTest, SetAndDeleteReturnsMemory) {
    size_t before = database_.GetTotalRamUsage();

    ASSERT_TRUE(database_.Set("key", "value").has_value());
    EXPECT_GT(database_.GetTotalRamUsage(), before);

    ASSERT_TRUE(database_.Del({"key"}).value > 0);
    database_.Flushdb();
    EXPECT_EQ(database_.GetTotalRamUsage(), empty_base_memory_);
}

TEST_F(MemoryTest, AppendAndDeleteReturnsMemory) {
    ASSERT_TRUE(database_.Set("key", "value").has_value());
    size_t before_append = database_.GetTotalRamUsage();

    ASSERT_TRUE(database_.Append("key", "extra_value").has_value());
    EXPECT_GT(database_.GetTotalRamUsage(), before_append);

    ASSERT_TRUE(database_.Del({"key"}).value > 0);
    database_.Flushdb();
    EXPECT_EQ(database_.GetTotalRamUsage(), empty_base_memory_);
}

TEST_F(MemoryTest, ListPushPopDeterministic) {
    size_t before_push = database_.GetTotalRamUsage();

    ASSERT_TRUE(database_.Lpush("list", {"item1"}).has_value());
    size_t after_first_push = database_.GetTotalRamUsage();
    EXPECT_GT(after_first_push, before_push);

    for (int i = 0; i < 20; ++i) {
        ASSERT_TRUE(database_.Lpush("list", {"item_" + std::to_string(i)}).has_value());
    }
    size_t after_many_pushes = database_.GetTotalRamUsage();
    EXPECT_GT(after_many_pushes, after_first_push);

    database_.Flushdb();
    EXPECT_EQ(database_.GetTotalRamUsage(), empty_base_memory_);
}

TEST_F(MemoryTest, SetAddRemoveDeterministic) {
    size_t before = database_.GetTotalRamUsage();

    ASSERT_TRUE(database_.Sadd("set", {"m1", "m2"}).has_value());
    size_t after_add = database_.GetTotalRamUsage();
    EXPECT_GT(after_add, before);

    database_.Flushdb();
    EXPECT_EQ(database_.GetTotalRamUsage(), empty_base_memory_);
}

TEST_F(MemoryTest, GeoAddDeleteDeterministic) {
    size_t before = database_.GetTotalRamUsage();

    ASSERT_TRUE(database_.Geoadd("geo", {{10.0, 20.0, "p1"}}).has_value());
    size_t after_add = database_.GetTotalRamUsage();
    EXPECT_GT(after_add, before);

    database_.Flushdb();
    EXPECT_EQ(database_.GetTotalRamUsage(), empty_base_memory_);
}

TEST_F(MemoryTest, MultipleTypesReturnMemory) {
    size_t before = database_.GetTotalRamUsage();

    database_.Set("str", "v");
    database_.Lpush("lst", {"i"});
    database_.Sadd("st", {"m"});

    EXPECT_GT(database_.GetTotalRamUsage(), before);

    database_.Flushdb();
    EXPECT_EQ(database_.GetTotalRamUsage(), empty_base_memory_);
}

TEST_F(MemoryTest, DeterministicAfterWrongType) {
    ASSERT_TRUE(database_.Set("key", "value").has_value());
    size_t after_set = database_.GetTotalRamUsage();

    auto res = database_.Lpush("key", {"item"});
    ASSERT_FALSE(res.has_value());

    size_t after_error = database_.GetTotalRamUsage();
    EXPECT_EQ(after_error, after_set);

    database_.Del({"key"});
    database_.Flushdb();
    EXPECT_EQ(database_.GetTotalRamUsage(), empty_base_memory_);
}

TEST_F(MemoryTest, OomOnLimitedMemory) {
    database_.ConfigSetMaxmemory(1);
    auto res = database_.Set("key", "value");
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), LogicError::kOom);
    EXPECT_EQ(database_.Dbsize().value, 0);

    database_.ConfigSetMaxmemory(0);
    ASSERT_TRUE(database_.Set("key", "value").has_value());
    EXPECT_EQ(database_.Dbsize().value, 1);
}

TEST_F(MemoryTest, NoMemoryLeakAfterRepeatedOperations) {
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(database_.Set("key", "value").has_value());
        ASSERT_TRUE(database_.Del({"key"}).value > 0);
        database_.Flushdb();
        EXPECT_EQ(database_.GetTotalRamUsage(), empty_base_memory_);
    }
}

TEST_F(MemoryTest, NoLeakWithMultipleTypesRepeated) {
    for (int i = 0; i < 5; ++i) {
        database_.Set("k" + std::to_string(i), "val");
        database_.Lpush("l" + std::to_string(i), {"item"});
        database_.Sadd("s" + std::to_string(i), {"mem"});

        database_.Del({"k" + std::to_string(i), "l" + std::to_string(i), "s" + std::to_string(i)});
        database_.Flushdb();
        EXPECT_EQ(database_.GetTotalRamUsage(), empty_base_memory_);
    }
}