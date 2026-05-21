#include <gtest/gtest.h>
#include <utility>
#include "../lib/includes/parsing/Parsing.h"

using namespace daikon::parsing;

class TokenizerTest : public ::testing::Test {
protected: 
    Tokenizer tokenizer_;
};

TEST_F(TokenizerTest, BasicCommands) {
    auto cmds_span = tokenizer_.GetTokens("PING");
    ASSERT_TRUE(cmds_span.has_value());
    auto cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].tokens.size(), 1);
    EXPECT_EQ(cmds[0].tokens[0], "PING");

    cmds_span = tokenizer_.GetTokens("SET key value");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].tokens.size(), 3);
    EXPECT_EQ(cmds[0].tokens[0], "SET");
    EXPECT_EQ(cmds[0].tokens[1], "key");
    EXPECT_EQ(cmds[0].tokens[2], "value");

    cmds_span = tokenizer_.GetTokens("LPUSH mylist a b c");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].tokens.size(), 5);
    EXPECT_EQ(cmds[0].tokens[0], "LPUSH");
    EXPECT_EQ(cmds[0].tokens[1], "mylist");
    EXPECT_EQ(cmds[0].tokens[2], "a");
    EXPECT_EQ(cmds[0].tokens[3], "b");
    EXPECT_EQ(cmds[0].tokens[4], "c");

    cmds_span = tokenizer_.GetTokens("   HSET   hash field   value   ");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].tokens.size(), 4);
    EXPECT_EQ(cmds[0].tokens[0], "HSET");
    EXPECT_EQ(cmds[0].tokens[1], "hash");
    EXPECT_EQ(cmds[0].tokens[2], "field");
    EXPECT_EQ(cmds[0].tokens[3], "value");
}

TEST_F(TokenizerTest, QuotedTokens) {
    auto cmds_span = tokenizer_.GetTokens("SET \"hello world\" 42");
    ASSERT_TRUE(cmds_span.has_value());
    auto cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].tokens.size(), 3);
    EXPECT_EQ(cmds[0].tokens[0], "SET");
    EXPECT_EQ(cmds[0].tokens[1], "hello world");
    EXPECT_EQ(cmds[0].tokens[2], "42");

    cmds_span = tokenizer_.GetTokens("SET \"\" empty");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].tokens.size(), 3);
    EXPECT_EQ(cmds[0].tokens[0], "SET");
    EXPECT_EQ(cmds[0].tokens[1], "");
    EXPECT_EQ(cmds[0].tokens[2], "empty");

    cmds_span = tokenizer_.GetTokens("LPUSH \"list name\" \"element 1\" \"element 2\"");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].tokens.size(), 4);
    EXPECT_EQ(cmds[0].tokens[0], "LPUSH");
    EXPECT_EQ(cmds[0].tokens[1], "list name");
    EXPECT_EQ(cmds[0].tokens[2], "element 1");
    EXPECT_EQ(cmds[0].tokens[3], "element 2");

    cmds_span = tokenizer_.GetTokens("GEOADD \"city:1\" 13.36 52.51 Berlin");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].tokens.size(), 5);
    EXPECT_EQ(cmds[0].tokens[0], "GEOADD");
    EXPECT_EQ(cmds[0].tokens[1], "city:1");
    EXPECT_EQ(cmds[0].tokens[2], "13.36");
    EXPECT_EQ(cmds[0].tokens[3], "52.51");
    EXPECT_EQ(cmds[0].tokens[4], "Berlin");
}

TEST_F(TokenizerTest, MultipleCommandsWithSemicolon) {
    auto cmds_span = tokenizer_.GetTokens("SET a 1; GET a");
    ASSERT_TRUE(cmds_span.has_value());
    auto cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 2);
    EXPECT_EQ(cmds[0].tokens.size(), 3);
    EXPECT_EQ(cmds[0].tokens[0], "SET");
    EXPECT_EQ(cmds[0].tokens[1], "a");
    EXPECT_EQ(cmds[0].tokens[2], "1");
    EXPECT_EQ(cmds[1].tokens.size(), 2);
    EXPECT_EQ(cmds[1].tokens[0], "GET");
    EXPECT_EQ(cmds[1].tokens[1], "a");

    cmds_span = tokenizer_.GetTokens("LPUSH list 1 2; LRANGE list 0 -1; DEL list");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 3);
    EXPECT_EQ(cmds[0].tokens.size(), 4);
    EXPECT_EQ(cmds[1].tokens.size(), 4);
    EXPECT_EQ(cmds[1].tokens[0], "LRANGE");
    EXPECT_EQ(cmds[2].tokens.size(), 2);
    EXPECT_EQ(cmds[2].tokens[0], "DEL");

    cmds_span = tokenizer_.GetTokens("SET \"hello;world\" x;GET \"hello;world\"");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 2);
    EXPECT_EQ(cmds[0].tokens[1], "hello;world");
    EXPECT_EQ(cmds[1].tokens[1], "hello;world");

    cmds_span = tokenizer_.GetTokens(";;;");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    EXPECT_TRUE(cmds.empty());

    cmds_span = tokenizer_.GetTokens("PING;PONG;ECHO hi");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 3);
    EXPECT_EQ(cmds[0].tokens[0], "PING");
    EXPECT_EQ(cmds[1].tokens[0], "PONG");
    EXPECT_EQ(cmds[2].tokens[0], "ECHO");
    EXPECT_EQ(cmds[2].tokens[1], "hi");
}

TEST_F(TokenizerTest, EdgeCases) {
    auto cmds_span = tokenizer_.GetTokens("");
    ASSERT_TRUE(cmds_span.has_value());
    auto cmds = cmds_span.value();
    EXPECT_TRUE(cmds.empty());

    cmds_span = tokenizer_.GetTokens("   ");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    EXPECT_TRUE(cmds.empty());

    cmds_span = tokenizer_.GetTokens(";");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    EXPECT_TRUE(cmds.empty());

    cmds_span = tokenizer_.GetTokens("SET a b;   ; GET c");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 2);
    EXPECT_EQ(cmds[0].tokens[0], "SET");
    EXPECT_EQ(cmds[1].tokens[0], "GET");

    cmds_span = tokenizer_.GetTokens("KEYS *");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].tokens.size(), 2);
    EXPECT_EQ(cmds[0].tokens[0], "KEYS");
    EXPECT_EQ(cmds[0].tokens[1], "*");
}

TEST_F(TokenizerTest, MixedSpacingAndNewlines) {
    auto cmds_span = tokenizer_.GetTokens("SADD set   a\tb\tc\r\nd");
    ASSERT_TRUE(cmds_span.has_value());
    auto cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].tokens.size(), 6);
    EXPECT_EQ(cmds[0].tokens[5], "d");

    cmds_span = tokenizer_.GetTokens("   ;   PING   ;   ");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 1);
    EXPECT_EQ(cmds[0].tokens[0], "PING");

    cmds_span = tokenizer_.GetTokens("CONFIG SET maxmemory 1048576; CONFIG GET maxmemory");
    ASSERT_TRUE(cmds_span.has_value());
    cmds = cmds_span.value();
    ASSERT_EQ(cmds.size(), 2);
    EXPECT_EQ(cmds[0].tokens[1], "SET");
    EXPECT_EQ(cmds[0].tokens[2], "maxmemory");
    EXPECT_EQ(cmds[1].tokens[1], "GET");
}

TEST_F(TokenizerTest, SyntaxErrors) {
    auto cmds_span = tokenizer_.GetTokens("SET \"unclosed quote");
    EXPECT_FALSE(cmds_span.has_value());

    cmds_span = tokenizer_.GetTokens("GET \"key with \"inner\" quote\"");
    EXPECT_TRUE(cmds_span.has_value());

    cmds_span = tokenizer_.GetTokens("SET \"hello\" \"world");
    EXPECT_FALSE(cmds_span.has_value());

    cmds_span = tokenizer_.GetTokens("\"quote at start\"");
    EXPECT_TRUE(cmds_span.has_value());
}