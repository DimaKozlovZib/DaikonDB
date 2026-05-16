#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "../lib/includes/Commands.h"
#include "../lib/includes/core/DaikonDB.h"
#include "../lib/includes/core/storage/database.h"

using namespace daikon;
using namespace daikon::core;

class DaikonDBTest : public ::testing::Test {
   protected:
    DaikonDatabase database{0};
    size_t empty_memory = 0;

    void SetUp() override { empty_memory = database.GetTotalRamUsage(); }
    void TearDown() override {
        database.Flushdb();
        ASSERT_EQ(database.GetTotalRamUsage(), empty_memory);
    }
};

TEST_F(DaikonDBTest, SetAndGet) {
    ASSERT_TRUE(database.Set("k", "hello").has_value());
    EXPECT_EQ(database.Get("k")->data, "hello");
    database.Set("k", "world");

    EXPECT_EQ(database.Get("k")->data, "world");
    EXPECT_FALSE(database.Get("no").has_value());
}

TEST_F(DaikonDBTest, StrlenAndAppend) {
    database.Set("k", "hi");
    EXPECT_EQ(database.Strlen("k")->value, 2);
    database.Append("k", " there");

    EXPECT_EQ(database.Get("k")->data, "hi there");
    EXPECT_FALSE(database.Strlen("no").has_value());
}

TEST_F(DaikonDBTest, ListOperations) {
    database.Lpush("l", {"b", "a"});
    database.Rpush("l", {"c"});

    EXPECT_EQ(database.Llen("l")->value, 3);
    EXPECT_EQ(database.Lindex("l", 0)->data, "a");

    database.Lset("l", 1, "x");
    database.Linsert("l", true, "c", "y");

    auto range = database.Lrange("l", 0, -1);
    EXPECT_EQ(range->elements, (std::vector<std::string>{"a", "x", "y", "c"}));

    database.Lpop("l", 1);
    database.Rpop("l", 1);
    EXPECT_EQ(database.Llen("l")->value, 2);
}

TEST_F(DaikonDBTest, SetOperations) {
    EXPECT_EQ(database.Sadd("s", {"a", "b"})->value, 2);
    EXPECT_EQ(database.Sadd("s", {"b", "c"})->value, 1);
    EXPECT_EQ(database.Scard("s")->value, 3);
    EXPECT_TRUE(database.Sismember("s", "a")->value);
    database.Srem("s", {"a"});
    auto members = database.Smembers("s")->elements;
    EXPECT_EQ(members.size(), 2u);
}

TEST_F(DaikonDBTest, SetMultiple) {
    database.Sadd("s1", {"a", "b"});
    database.Sadd("s2", {"b", "c"});
    auto un = database.Sunion({"s1", "s2"})->elements;
    std::sort(un.begin(), un.end());
    EXPECT_EQ(un, (std::vector<std::string>{"a", "b", "c"}));
    auto in = database.Sinter({"s1", "s2"})->elements;
    EXPECT_EQ(in, (std::vector<std::string>{"b"}));
    database.Smove("s1", "s2", "a");
    EXPECT_EQ(database.Scard("s2")->value, 3);
}

TEST_F(DaikonDBTest, GeoOperations) {
    database.Geoadd("g", {{0, 0, "a"}, {0, 1, "b"}});
    auto pos = *database.Geopos("g", {"a", "b"});
    EXPECT_TRUE(pos[0].has_value());
    EXPECT_NEAR(*database.Geodist("g", "a", "b", "km"), 111.32, 1.0);
    auto sr = database.Geosearch("g", 0, 0, 150, "km", true)->elements;
    EXPECT_EQ(sr.size(), 2u);
    database.Geosearchstore("g2", "g", 0, 0, 150, "km", true);
    EXPECT_TRUE((*database.Geopos("g2", {"a"}))[0].has_value());
}

TEST_F(DaikonDBTest, TypeAndDel) {
    database.Set("a", "1");
    database.Lpush("b", {"2"});
    EXPECT_EQ(database.Type("a").data, "string");
    EXPECT_EQ(database.Type("b").data, "list");
    EXPECT_EQ(database.Type("no").data, "none");
    EXPECT_EQ(database.Del({"a", "b", "x"}).value, 2);
    EXPECT_EQ(database.Exists({"a", "b"}).value, 0);
}

TEST_F(DaikonDBTest, KeysMemoryExpire) {
    database.Set("x", "1");
    database.Set("y", "2");
    auto keys = database.Keys("*").elements;
    EXPECT_EQ(keys.size(), 2u);
    EXPECT_GT(database.MemoryUsage("x").value, 0);
    database.Expire("x", 10);
    EXPECT_GT(database.Ttl("x").value, 0);
    EXPECT_EQ(database.Ttl("y").value, -1);
    database.Flushdb();
    EXPECT_EQ(database.Dbsize().value, 0);
}