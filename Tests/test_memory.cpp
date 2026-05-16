#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

#include "../lib/includes/Commands.h"
#include "../lib/includes/core/DaikonDB.h"
#include "../lib/includes/core/storage/database.h"

using namespace daikon;
using namespace daikon::core;

class MemoryTest : public ::testing::Test {
   protected:
    DaikonDatabase database{0};
    size_t empty_base_memory = 0;

    void SetUp() override {
        empty_base_memory = database.GetTotalRamUsage();
    }

    void TearDown() override {
        database.Flushdb();
    }
};

TEST_F(MemoryTest, InitialState) {
    EXPECT_EQ(database.Dbsize().value, 0);
    EXPECT_EQ(database.ConfigGetMaxmemory().value, 0);
}

TEST_F(MemoryTest, MemoryUsageForKey) {
    size_t before_set = database.GetTotalRamUsage();
    
    auto set_res = database.Set("mykey", "hello world");
    ASSERT_TRUE(set_res.has_value());
    ASSERT_TRUE(set_res->success);

    auto usage = database.MemoryUsage("mykey");
    EXPECT_GT(usage.value, 0);

    size_t after_set = database.GetTotalRamUsage();
    EXPECT_GT(after_set, before_set);

    database.Del({"mykey"});
    database.Flushdb();
    EXPECT_EQ(database.GetTotalRamUsage(), empty_base_memory);
}

TEST_F(MemoryTest, SetAndDeleteReturnsMemory) {
    size_t before = database.GetTotalRamUsage();

    ASSERT_TRUE(database.Set("key", "value").has_value());
    EXPECT_GT(database.GetTotalRamUsage(), before);

    ASSERT_TRUE(database.Del({"key"}).value > 0);
    database.Flushdb();
    EXPECT_EQ(database.GetTotalRamUsage(), empty_base_memory);
}

TEST_F(MemoryTest, AppendAndDeleteReturnsMemory) {
    ASSERT_TRUE(database.Set("key", "value").has_value());
    size_t before_append = database.GetTotalRamUsage();

    ASSERT_TRUE(database.Append("key", "extra_value").has_value());
    EXPECT_GT(database.GetTotalRamUsage(), before_append);

    ASSERT_TRUE(database.Del({"key"}).value > 0);
    database.Flushdb();
    EXPECT_EQ(database.GetTotalRamUsage(), empty_base_memory);
}

TEST_F(MemoryTest, ListPushPopDeterministic) {
    size_t before_push = database.GetTotalRamUsage();

    ASSERT_TRUE(database.Lpush("list", {"item1"}).has_value());
    size_t after_first_push = database.GetTotalRamUsage();
    EXPECT_GT(after_first_push, before_push);

    for (int i = 0; i < 20; ++i) {
        ASSERT_TRUE(database.Lpush("list", {"item_" + std::to_string(i)}).has_value());
    }
    size_t after_many_pushes = database.GetTotalRamUsage();
    EXPECT_GT(after_many_pushes, after_first_push);

    database.Flushdb();
    EXPECT_EQ(database.GetTotalRamUsage(), empty_base_memory);
}

TEST_F(MemoryTest, SetAddRemoveDeterministic) {
    size_t before = database.GetTotalRamUsage();

    ASSERT_TRUE(database.Sadd("set", {"m1", "m2"}).has_value());
    size_t after_add = database.GetTotalRamUsage();
    EXPECT_GT(after_add, before);

    database.Flushdb();
    EXPECT_EQ(database.GetTotalRamUsage(), empty_base_memory);
}

TEST_F(MemoryTest, GeoAddDeleteDeterministic) {
    size_t before = database.GetTotalRamUsage();

    ASSERT_TRUE(database.Geoadd("geo", {{10.0, 20.0, "p1"}}).has_value());
    size_t after_add = database.GetTotalRamUsage();
    EXPECT_GT(after_add, before);

    database.Flushdb();
    EXPECT_EQ(database.GetTotalRamUsage(), empty_base_memory);
}

TEST_F(MemoryTest, MultipleTypesReturnMemory) {
    size_t before = database.GetTotalRamUsage();

    database.Set("str", "v");
    database.Lpush("lst", {"i"});
    database.Sadd("st", {"m"});

    EXPECT_GT(database.GetTotalRamUsage(), before);

    database.Flushdb();
    EXPECT_EQ(database.GetTotalRamUsage(), empty_base_memory);
}

TEST_F(MemoryTest, DeterministicAfterWrongType) {
    ASSERT_TRUE(database.Set("key", "value").has_value());
    size_t after_set = database.GetTotalRamUsage();

    auto res = database.Lpush("key", {"item"});
    ASSERT_FALSE(res.has_value());
    
    size_t after_error = database.GetTotalRamUsage();
    EXPECT_EQ(after_error, after_set);

    database.Del({"key"});
    database.Flushdb();
    EXPECT_EQ(database.GetTotalRamUsage(), empty_base_memory);
}

TEST_F(MemoryTest, OomOnLimitedMemory) {
    database.ConfigSetMaxmemory(1);
    auto res = database.Set("key", "value");
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), LogicError::kOom);
    EXPECT_EQ(database.Dbsize().value, 0);

    database.ConfigSetMaxmemory(0);
    ASSERT_TRUE(database.Set("key", "value").has_value());
    EXPECT_EQ(database.Dbsize().value, 1);
}

TEST_F(MemoryTest, NoMemoryLeakAfterRepeatedOperations) {
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(database.Set("key", "value").has_value());
        ASSERT_TRUE(database.Del({"key"}).value > 0);
        database.Flushdb();
        EXPECT_EQ(database.GetTotalRamUsage(), empty_base_memory);
    }
}

TEST_F(MemoryTest, NoLeakWithMultipleTypesRepeated) {
    for (int i = 0; i < 5; ++i) {
        database.Set("k" + std::to_string(i), "val");
        database.Lpush("l" + std::to_string(i), {"item"});
        database.Sadd("s" + std::to_string(i), {"mem"});
        
        database.Del({"k" + std::to_string(i), "l" + std::to_string(i), "s" + std::to_string(i)});
        database.Flushdb();
        EXPECT_EQ(database.GetTotalRamUsage(), empty_base_memory);
    }
}