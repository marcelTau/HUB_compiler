#include <gtest/gtest.h>

#include "../src/lexer.h"

using enum TokenType;
using TokenList = Lexer::TokenList;
using namespace std::string_view_literals;

template<>
struct fmt::formatter<TokenList> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx);

    template<typename FormatContext>
    auto format(const TokenList& tl, FormatContext& ctx);
};

template<typename ParseContext>
constexpr auto fmt::formatter<TokenList>::parse(ParseContext& ctx) {
    return ctx.begin();
}

template<typename FormatContext>
auto fmt::formatter<TokenList>::format(const TokenList& tl, FormatContext& ctx) {
    std::stringstream ss;

    for (const auto& token : tl) {
        ss << fmt::format("{}", token);
    }
    return fmt::format_to(ctx.out(), "{}", ss.str());
}

static auto setup(const std::string_view code) -> TokenList {
    Lexer l(code);
    return l.lex();
}

TEST(lexer, parens) {
    const auto expected = Lexer::TokenList {
        Token { .ttype = LeftParen, .lexeme = "(", .position = { 1, 1 } },
        Token { .ttype = RightParen, .lexeme = ")", .position = { 1, 2 } },
        Token { .ttype = Eof, .lexeme = "", .position = { 1, 2 } },
    };

    const auto tokens = setup("()"sv);

    EXPECT_EQ(tokens, expected) << fmt::format("\nGot:      {}\nExpected: {}\n", tokens, expected);;
}

TEST(lexer, Inumbers) {
    Lexer l("10 + 20"sv);
    const auto tokens = l.lex();
    const auto expected = Lexer::TokenList {
        Token { .ttype = INumber, .lexeme = "10", .position = { 1, 2 } },
        Token { .ttype = Plus, .lexeme = "+", .position = { 1, 4 } },
        Token { .ttype = INumber, .lexeme = "20", .position = { 1, 7 } },
        Token { .ttype = Eof, .lexeme = "", .position = { 1, 7 } },
    };

    EXPECT_EQ(tokens, expected) << fmt::format("\nGot:      {}\nExpected: {}\n", tokens, expected);;
}
TEST(lexer, Dnumbers) {
    Lexer l("10.5"sv);
    const auto tokens = l.lex();
    const auto expected = Lexer::TokenList {
        Token { .ttype = DNumber, .lexeme = "10.5", .position = { 1, 4 } },
        Token { .ttype = Eof, .lexeme = "", .position = { 1, 4 } },
    };

    EXPECT_EQ(tokens, expected) << fmt::format("\nGot:      {}\nExpected: {}\n", tokens, expected);;
}

TEST(lexer, Identifier) {
    Lexer l("a"sv);
    const auto tokens = l.lex();
    const auto expected = Lexer::TokenList {
        Token { .ttype = Identifier, .lexeme = "a", .position = { 1, 1 } },
        Token { .ttype = Eof, .lexeme = "", .position = { 1, 1 } },
    };

    EXPECT_EQ(tokens, expected) << fmt::format("\nGot:      {}\nExpected: {}\n", tokens, expected);;
}
