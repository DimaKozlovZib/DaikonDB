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
        ASSERT_EQ(database.Dbsize().value, 0);
        empty_base_memory = database.Dbsize().value;
    }

    void TearDown() override {
        database.Flushdb();
        ASSERT_EQ(database.Dbsize().value, empty_base_memory);
    }
};

TEST_F(MemoryTest, InitialState) {
    EXPECT_EQ(database.Dbsize().value, 0);
    EXPECT_EQ(database.ConfigGetMaxmemory().value, 0);
}

TEST_F(MemoryTest, MemoryUsageForKey) {
    auto set_res = database.Set("mykey", "hello world");
    ASSERT_TRUE(set_res.has_value());
    ASSERT_TRUE(set_res->success);

    auto usage = database.MemoryUsage("mykey");
    EXPECT_GT(usage.value, 0);

    auto missing = database.MemoryUsage("no_such_key");
    EXPECT_EQ(missing.value, 0);
}

TEST_F(MemoryTest, ConfigMaxmemory) {
    database.ConfigSetMaxmemory(1024);
    EXPECT_EQ(database.ConfigGetMaxmemory().value, 1024);
    EXPECT_EQ(database.GetMaxMemory(), 1024);
    database.ConfigSetMaxmemory(0);
    EXPECT_EQ(database.ConfigGetMaxmemory().value, 0);
}

TEST_F(MemoryTest, Dbsize) {
    EXPECT_EQ(database.Dbsize().value, 0);
    database.Set("k1", "v1");
    EXPECT_EQ(database.Dbsize().value, 1);
    database.Set("k2", "v2");
    EXPECT_EQ(database.Dbsize().value, 2);
    database.Del({"k1"});
    EXPECT_EQ(database.Dbsize().value, 1);
    database.Flushdb();
    EXPECT_EQ(database.Dbsize().value, 0);
}

TEST_F(MemoryTest, ExpireAndTtl) {
    auto set_res = database.Set("temp", "value");
    ASSERT_TRUE(set_res.has_value());

    auto expire_res = database.Expire("temp", 10);
    ASSERT_TRUE(expire_res.success);
    auto ttl = database.Ttl("temp");
    EXPECT_GT(ttl.value, 0);
    EXPECT_LE(ttl.value, 10);

    size_t mem_before = database.GetTotalRamUsage();
    database.Expire("temp", 20);
    EXPECT_EQ(database.GetTotalRamUsage(), mem_before);
}

TEST_F(MemoryTest, SetAndDeleteReturnsMemory) {
    size_t initial = empty_base_memory;
    auto set_res = database.Set("key", "hello");
    ASSERT_TRUE(set_res.has_value());
    size_t after_set = database.GetTotalRamUsage();
    EXPECT_GT(after_set, initial);

    ASSERT_TRUE(database.Del({"key"}).value > 0);
    EXPECT_EQ(database.GetTotalRamUsage(), initial);
}

TEST_F(MemoryTest, AppendAndDeleteReturnsMemory) {
    size_t initial = empty_base_memory;
    database.Set("key", "hello");
    size_t after_set = database.GetTotalRamUsage();
    EXPECT_GT(after_set, initial);

    auto append_res = database.Append("key", " world");
    ASSERT_TRUE(append_res.has_value());
    size_t after_append = database.GetTotalRamUsage();

    EXPECT_GE(after_append, after_set);

    database.Del({"key"});
    EXPECT_EQ(database.GetTotalRamUsage(), initial);
}

TEST_F(MemoryTest, ListPushPopDeterministic) {
    size_t initial = empty_base_memory;

    ASSERT_TRUE(database.Lpush("mylist", {"a", "b", "c"}).has_value());
    size_t after_push = database.GetTotalRamUsage();
    EXPECT_GT(after_push, initial);

    database.Del({"mylist"});
    EXPECT_EQ(database.GetTotalRamUsage(), initial);

    ASSERT_TRUE(database.Lpush("mylist", {"x"}).has_value());
    size_t before_second_push = database.GetTotalRamUsage();
    ASSERT_TRUE(database.Rpush("mylist", {"y", "z"}).has_value());
    size_t after_second_push = database.GetTotalRamUsage();
    EXPECT_GT(after_second_push, before_second_push);

    database.Del({"mylist"});
    EXPECT_EQ(database.GetTotalRamUsage(), initial);
}

TEST_F(MemoryTest, SetAddRemoveDeterministic) {
    size_t initial = empty_base_memory;

    ASSERT_TRUE(database.Sadd("myset", {"a", "b", "c"}).has_value());
    size_t after_add = database.GetTotalRamUsage();
    EXPECT_GT(after_add, initial);

    database.Srem("myset", {"a", "b", "c"});
    size_t after_remove = database.GetTotalRamUsage();
    EXPECT_GT(after_add, after_remove);

    database.Del({"myset"});
    EXPECT_EQ(database.GetTotalRamUsage(), initial);
}

TEST_F(MemoryTest, GeoAddDeleteDeterministic) {
    size_t initial = empty_base_memory;

    std::vector<commands::GeoPoint> points = {
        {10.0, 20.0, "p1"},
        {10.1, 20.1, "p2"}};
    ASSERT_TRUE(database.Geoadd("geo", points).has_value());
    size_t after_add = database.GetTotalRamUsage();
    EXPECT_GT(after_add, initial);

    database.Del({"geo"});
    EXPECT_EQ(database.GetTotalRamUsage(), initial);
}

TEST_F(MemoryTest, MultipleTypesReturnMemory) {
    size_t initial = empty_base_memory;

    database.Set("str", "value");
    database.Lpush("list", {"1", "2"});
    database.Sadd("set", {"a", "b"});
    database.Geoadd("geo", {{0.0, 0.0, "p"}});

    size_t after_all = database.GetTotalRamUsage();
    EXPECT_GT(after_all, initial);

    database.Del({"str", "list", "set", "geo"});
    EXPECT_EQ(database.GetTotalRamUsage(), initial);
}

TEST_F(MemoryTest, DeterministicAfterWrongType) {
    size_t initial = empty_base_memory;

    database.Set("key", "string_value");
    size_t after_set = database.GetTotalRamUsage();

    auto res = database.Lpush("key", {"a"});
    ASSERT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), LogicError::kWrongType);

    size_t after_error = database.GetTotalRamUsage();
    EXPECT_EQ(after_error, after_set);

    database.Del({"key"});
    EXPECT_EQ(database.GetTotalRamUsage(), initial);
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
    size_t initial = empty_base_memory;

    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(database.Set("key", "value").has_value());
        ASSERT_TRUE(database.Del({"key"}).value > 0);
        EXPECT_EQ(database.GetTotalRamUsage(), initial);
    }
}

TEST_F(MemoryTest, NoLeakWithMultipleTypesRepeated) {
    size_t initial = empty_base_memory;

    for (int i = 0; i < 5; ++i) {
        database.Set("k" + std::to_string(i), "val");
    }
    for (int i = 0; i < 5; ++i) {
        database.Del({"k" + std::to_string(i)});
    }
    EXPECT_EQ(database.GetTotalRamUsage(), initial);

    ASSERT_TRUE(database.Lpush("list", {"a", "b", "c"}).has_value());
    database.Del({"list"});
    EXPECT_EQ(database.GetTotalRamUsage(), initial);

    ASSERT_TRUE(database.Sadd("set", {"a", "b"}).has_value());
    database.Del({"set"});
    EXPECT_EQ(database.GetTotalRamUsage(), initial);
}