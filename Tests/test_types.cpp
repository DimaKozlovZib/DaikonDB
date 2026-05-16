#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <string_view>
#include <vector>
#include <string>
#include <optional>
#include <cstddef>
#include <algorithm>
#include <thread>
#include <chrono>

#include "../lib/includes/core/storage/database.h"
#include "../lib/includes/core/types/Geo.h"
#include "../lib/includes/core/types/List.h"
#include "../lib/includes/core/types/Set.h"
#include "../lib/includes/core/types/String.h"

using namespace daikon::core;
using namespace daikon::core::types;

class TypesTest : public ::testing::Test {
   protected:
    DBase db_;
};

TEST_F(TypesTest, StringObject_CreateAndGet) {
    auto str = StringObject(&db_, "hello");
    EXPECT_EQ(str.Get(), "hello");
    EXPECT_EQ(str.Strlen(), 5);
}

TEST_F(TypesTest, StringObject_Append) {
    auto str = StringObject(&db_, "hello");
    str.Append(" world");
    EXPECT_EQ(str.Get(), "hello world");
    EXPECT_EQ(str.Strlen(), 11);
}

TEST_F(TypesTest, StringObject_EstimateCreate) {
    int64_t est = StringObject::EstimateCreate("hello");
    EXPECT_GT(est, 5);
    EXPECT_EQ(est, sizeof(StringObject) + 5 + 32);
}

TEST_F(TypesTest, ListObject_PushLeftRight) {
    ListObject list(&db_);
    list.PushLeft({"a", "b"});
    list.PushRight({"c", "d"});
    EXPECT_EQ(list.Len(), 4);
    auto range = list.GetRange(0, -1);
    std::vector<std::string_view> expected = {"b", "a", "c", "d"};
    EXPECT_EQ(range, expected);
}

TEST_F(TypesTest, ListObject_Pop) {
    ListObject list(&db_);
    list.PushRight({"1", "2", "3", "4"});
    auto left = list.PopLeft(2);
    EXPECT_EQ(left, (std::vector<std::string>{"1", "2"}));
    EXPECT_EQ(list.Len(), 2);
    auto right = list.PopRight(1);
    EXPECT_EQ(right, (std::vector<std::string>{"4"}));
    EXPECT_EQ(list.Len(), 1);
}

TEST_F(TypesTest, ListObject_GetByIndex) {
    ListObject list(&db_);
    list.PushRight({"a", "b", "c"});
    EXPECT_EQ(list.GetByIndex(0), "a");
    EXPECT_EQ(list.GetByIndex(-1), "c");
    EXPECT_EQ(list.GetByIndex(5), std::nullopt);
}

TEST_F(TypesTest, ListObject_Set) {
    ListObject list(&db_);
    list.PushRight({"a", "b"});
    list.Set(0, "new");
    EXPECT_EQ(list.GetByIndex(0), "new");
}

TEST_F(TypesTest, ListObject_Insert) {
    ListObject list(&db_);
    list.PushRight({"a", "c"});
    int64_t res = list.Insert(true, "c", "b");
    EXPECT_EQ(res, 3);
    auto range = list.GetRange(0, -1);
    std::vector<std::string_view> expected = {"a", "b", "c"};
    EXPECT_EQ(range, expected);
}

TEST_F(TypesTest, ListObject_MemoryView_EstimatePush) {
    ListObject list(&db_);
    auto view = list.GetMemoryView();
    std::vector<std::string_view> vals = {"hello", "world"};
    int64_t est = view.EstimatePush(vals);
    EXPECT_EQ(est, (5 + 32) + (5 + 32));
}

TEST_F(TypesTest, SetObject_AddAndContains) {
    SetObject set(&db_);
    size_t added = set.Add({"a", "b", "c"});
    EXPECT_EQ(added, 3);
    EXPECT_TRUE(set.IsMember("a"));
    EXPECT_FALSE(set.IsMember("d"));
    EXPECT_EQ(set.Card(), 3);
}

TEST_F(TypesTest, SetObject_Remove) {
    SetObject set(&db_);
    set.Add({"a", "b", "c"});
    size_t removed = set.Remove({"a", "c"});
    EXPECT_EQ(removed, 2);
    EXPECT_FALSE(set.IsMember("a"));
    EXPECT_TRUE(set.IsMember("b"));
}

TEST_F(TypesTest, SetObject_AllMembers) {
    SetObject set(&db_);
    set.Add({"z", "a", "m"});
    auto members = set.AllMembers();
    std::sort(members.begin(), members.end());
    EXPECT_EQ(members, (std::vector<std::string>{"a", "m", "z"}));
}

TEST_F(TypesTest, SetObject_UnionSets) {
    SetObject set1(&db_), set2(&db_);
    set1.Add({"a", "b"});
    set2.Add({"b", "c"});
    auto result = SetObject::UnionSets({&set1, &set2});
    std::sort(result.begin(), result.end());
    EXPECT_EQ(result, (std::vector<std::string>{"a", "b", "c"}));
}

TEST_F(TypesTest, SetObject_InterSets) {
    SetObject set1(&db_), set2(&db_);
    set1.Add({"a", "b", "c"});
    set2.Add({"b", "c", "d"});
    auto result = SetObject::InterSets({&set1, &set2});
    std::sort(result.begin(), result.end());
    EXPECT_EQ(result, (std::vector<std::string>{"b", "c"}));
}

TEST_F(TypesTest, SetObject_DiffSets) {
    SetObject set1(&db_), set2(&db_);
    set1.Add({"a", "b", "c"});
    set2.Add({"b"});
    auto result = SetObject::DiffSets({&set1, &set2});
    std::sort(result.begin(), result.end());
    EXPECT_EQ(result, (std::vector<std::string>{"a", "c"}));
}

TEST_F(TypesTest, SetObject_MoveMember) {
    SetObject src(&db_), dst(&db_);
    src.Add({"x", "y"});
    dst.Add({"z"});
    bool ok = src.MoveMember(dst, "x");
    EXPECT_TRUE(ok);
    EXPECT_FALSE(src.IsMember("x"));
    EXPECT_TRUE(dst.IsMember("x"));
    EXPECT_EQ(dst.Card(), 2);
}

TEST_F(TypesTest, GeoObject_AddAndPos) {
    GeoObject geo(&db_);
    geo.Add(10.0, 20.0, "point1");
    geo.Add(11.0, 21.0, "point2");
    auto pos = geo.Pos({"point1", "point3", "point2"});
    ASSERT_EQ(pos.size(), 3);
    EXPECT_TRUE(pos[0].has_value());
    EXPECT_DOUBLE_EQ(pos[0]->first, 10.0);
    EXPECT_DOUBLE_EQ(pos[0]->second, 20.0);
    EXPECT_FALSE(pos[1].has_value());
    EXPECT_TRUE(pos[2].has_value());
}

TEST_F(TypesTest, GeoObject_Dist) {
    GeoObject geo(&db_);
    geo.Add(0.0, 0.0, "a");
    geo.Add(0.0, 1.0, "b");
    auto dist = geo.Dist("a", "b", "km");
    ASSERT_TRUE(dist.has_value());
    EXPECT_NEAR(*dist, 111.32, 1.0);
}

TEST_F(TypesTest, GeoObject_Search) {
    GeoObject geo(&db_);
    geo.Add(0.0, 0.0, "center");
    geo.Add(0.0, 0.5, "near");
    geo.Add(0.0, 2.0, "far");
    auto result = geo.Search(0.0, 0.0, 100.0, "km", true);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[1], "near");
    EXPECT_EQ(result[0], "center");
}

TEST_F(TypesTest, GeoObject_SearchPoints) {
    GeoObject geo(&db_);
    geo.Add(10.0, 20.0, "p1");
    geo.Add(10.1, 20.1, "p2");
    auto res = geo.SearchPoints(10.0, 20.0, 50.0, "km");
    ASSERT_EQ(res.size(), 2);

    EXPECT_EQ(res[0].second.longitude, 10.1);
    EXPECT_EQ(res[0].second.latitude, 20.1);
}

TEST_F(TypesTest, BaseDataObject_TtlExpire) {
    StringObject str(&db_, "ttl_test");
    EXPECT_FALSE(str.HasTtl());
    EXPECT_FALSE(str.IsExpired());
    str.SetTTL(std::chrono::seconds(1));
    EXPECT_TRUE(str.HasTtl());
    EXPECT_FALSE(str.IsExpired());

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    EXPECT_TRUE(str.IsExpired());
}

TEST_F(TypesTest, MemoryViews_Coverage) {
    StringObject str(&db_, "test");
    auto sv = str.GetMemoryView();
    sv.EstimateSet("new");
    sv.EstimateAppend("_suffix");
    StringObject::EstimateCreate("test");

    ListObject list(&db_);
    list.PushRight({"a"});
    auto lv = list.GetMemoryView();
    lv.EstimatePush({"b"});
    lv.EstimateSet(0, "cc");
    lv.EstimateInsert("dd");
    ListObject::EstimateCreate();

    SetObject set(&db_);
    set.Add({"x"});
    auto setv = set.GetMemoryView();
    setv.EstimateAdd({"y"});
    setv.EstimateMove("z");
    SetObject::EstimateCreate();

    GeoObject geo(&db_);
    geo.Add(0, 0, "p");
    auto gv = geo.GetMemoryView();
    gv.EstimateAdd("q");
    GeoObject::EstimateCreate();
    SUCCEED();
}