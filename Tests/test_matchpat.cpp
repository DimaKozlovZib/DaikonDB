#include <gtest/gtest.h>
#include <string_view>

namespace daikon {
   extern bool MatchGlobPattern(std::string_view pattern, std::string_view text); 
}  // namespace daikon

TEST(GlobTest, Exact) {
    EXPECT_TRUE(daikon::MatchGlobPattern("hello", "hello"));
    EXPECT_FALSE(daikon::MatchGlobPattern("hello", "hell"));
    EXPECT_FALSE(daikon::MatchGlobPattern("hello", "helloo"));
}

TEST(GlobTest, Star) {
    EXPECT_TRUE(daikon::MatchGlobPattern("*", ""));
    EXPECT_TRUE(daikon::MatchGlobPattern("*", "a"));
    EXPECT_TRUE(daikon::MatchGlobPattern("h*o", "ho"));
    EXPECT_TRUE(daikon::MatchGlobPattern("h*o", "hello"));
    EXPECT_TRUE(daikon::MatchGlobPattern("*x*", "x"));
    EXPECT_TRUE(daikon::MatchGlobPattern("*x*", "axb"));
    EXPECT_FALSE(daikon::MatchGlobPattern("*x*", "ab"));
}

TEST(GlobTest, Question) {
    EXPECT_TRUE(daikon::MatchGlobPattern("h?llo", "hello"));
    EXPECT_FALSE(daikon::MatchGlobPattern("h?llo", "hllo"));
    EXPECT_TRUE(daikon::MatchGlobPattern("?", "a"));
    EXPECT_FALSE(daikon::MatchGlobPattern("?", ""));
}

TEST(GlobTest, CharClass) {
    EXPECT_TRUE(daikon::MatchGlobPattern("[abc]", "a"));
    EXPECT_FALSE(daikon::MatchGlobPattern("[abc]", "d"));
    EXPECT_TRUE(daikon::MatchGlobPattern("[a-z]", "m"));
    EXPECT_FALSE(daikon::MatchGlobPattern("[a-z]", "9"));
    EXPECT_TRUE(daikon::MatchGlobPattern("[0-9]", "5"));
}

TEST(GlobTest, NegateClass) {
    EXPECT_TRUE(daikon::MatchGlobPattern("[^abc]", "d"));
    EXPECT_FALSE(daikon::MatchGlobPattern("[^abc]", "a"));
}

TEST(GlobTest, Escape) {
    EXPECT_TRUE(daikon::MatchGlobPattern("\\*", "*"));
    EXPECT_TRUE(daikon::MatchGlobPattern("\\?", "?"));
    EXPECT_FALSE(daikon::MatchGlobPattern("\\*", "a"));
    EXPECT_FALSE(daikon::MatchGlobPattern("\\?", "b"));
}

TEST(GlobTest, Complex) {
    EXPECT_TRUE(daikon::MatchGlobPattern("*.txt", "readme.txt"));
    EXPECT_TRUE(daikon::MatchGlobPattern("a?c*", "abcdef"));
    EXPECT_TRUE(daikon::MatchGlobPattern("[a-z]*[0-9]", "test5"));
    EXPECT_FALSE(daikon::MatchGlobPattern("[a-z]*[0-9]", "test"));
    EXPECT_TRUE(daikon::MatchGlobPattern("a*b?[0-9]", "axyzbc5"));
}