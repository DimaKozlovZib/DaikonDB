#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "../lib/includes/core/DaikonDB.h"

using namespace daikon;
using namespace daikon::core;

class DaikonDBTest : public ::testing::Test {
   protected:
    DaikonDatabase database_{0};
    size_t empty_memory_ = 0;

    void SetUp() override { empty_memory_ = database_.GetTotalRamUsage(); }
    void TearDown() override {
        database_.Flushdb();
        ASSERT_EQ(database_.GetTotalRamUsage(), empty_memory_);
    }
};

TEST_F(DaikonDBTest, SetAndGet) {
    ASSERT_TRUE(database_.Set("k", "hello").has_value());
    EXPECT_EQ(database_.Get("k")->data, "hello");
    database_.Set("k", "world");

    EXPECT_EQ(database_.Get("k")->data, "world");
    EXPECT_FALSE(database_.Get("no").has_value());
}

TEST_F(DaikonDBTest, StrlenAndAppend) {
    database_.Set("k", "hi");
    EXPECT_EQ(database_.Strlen("k")->value, 2);
    database_.Append("k", " there");

    EXPECT_EQ(database_.Get("k")->data, "hi there");
    EXPECT_FALSE(database_.Strlen("no").has_value());
}

TEST_F(DaikonDBTest, ListOperations) {
    database_.Lpush("l", {"b", "a"});
    database_.Rpush("l", {"c"});

    EXPECT_EQ(database_.Llen("l")->value, 3);
    EXPECT_EQ(database_.Lindex("l", 0)->data, "a");

    database_.Lset("l", 1, "x");
    database_.Linsert("l", true, "c", "y");

    auto range = database_.Lrange("l", 0, -1);
    EXPECT_EQ(range->elements, (std::vector<std::string>{"a", "x", "y", "c"}));

    database_.Lpop("l", 1);
    database_.Rpop("l", 1);
    EXPECT_EQ(database_.Llen("l")->value, 2);
}

TEST_F(DaikonDBTest, SetOperations) {
    EXPECT_EQ(database_.Sadd("s", {"a", "b"})->value, 2);
    EXPECT_EQ(database_.Sadd("s", {"b", "c"})->value, 1);
    EXPECT_EQ(database_.Scard("s")->value, 3);
    EXPECT_TRUE(database_.Sismember("s", "a")->value);
    database_.Srem("s", {"a"});
    auto members = database_.Smembers("s")->elements;
    EXPECT_EQ(members.size(), 2u);
}

TEST_F(DaikonDBTest, SetMultiple) {
    database_.Sadd("s1", {"a", "b"});
    database_.Sadd("s2", {"b", "c"});
    auto un = database_.Sunion({"s1", "s2"})->elements;
    std::sort(un.begin(), un.end());
    EXPECT_EQ(un, (std::vector<std::string>{"a", "b", "c"}));
    auto in = database_.Sinter({"s1", "s2"})->elements;
    EXPECT_EQ(in, (std::vector<std::string>{"b"}));
    database_.Smove("s1", "s2", "a");
    EXPECT_EQ(database_.Scard("s2")->value, 3);
}

TEST_F(DaikonDBTest, GeoOperations) {
    database_.Geoadd("g", {{0, 0, "a"}, {0, 1, "b"}});
    auto pos = *database_.Geopos("g", {"a", "b"});
    EXPECT_TRUE(pos[0].has_value());
    EXPECT_NEAR(*database_.Geodist("g", "a", "b", "km"), 111.32, 1.0);
    auto sr = database_.Geosearch("g", 0, 0, 150, "km", true)->elements;
    EXPECT_EQ(sr.size(), 2u);
    database_.Geosearchstore("g2", "g", 0, 0, 150, "km", true);
    EXPECT_TRUE((*database_.Geopos("g2", {"a"}))[0].has_value());
}

TEST_F(DaikonDBTest, TypeAndDel) {
    database_.Set("a", "1");
    database_.Lpush("b", {"2"});
    EXPECT_EQ(database_.Type("a").data, "string");
    EXPECT_EQ(database_.Type("b").data, "list");
    EXPECT_EQ(database_.Type("no").data, "none");
    EXPECT_EQ(database_.Del({"a", "b", "x"}).value, 2);
    EXPECT_EQ(database_.Exists({"a", "b"}).value, 0);
}

TEST_F(DaikonDBTest, KeysMemoryExpire) {
    database_.Set("x", "1");
    database_.Set("y", "2");
    auto keys = database_.Keys("*").elements;
    EXPECT_EQ(keys.size(), 2u);
    EXPECT_GT(database_.MemoryUsage("x").value, 0);
    database_.Expire("x", 10);
    EXPECT_GT(database_.Ttl("x").value, 0);
    EXPECT_EQ(database_.Ttl("y").value, -1);
    database_.Flushdb();
    EXPECT_EQ(database_.Dbsize().value, 0);
}