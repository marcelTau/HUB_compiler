#include <gtest/gtest.h>
#include <memory>
#include "parser.h"

using StmtList = std::vector<std::unique_ptr<Statements::Statement>>;

static auto setup(const std::string_view code) -> StmtList {
    Lexer l(code);
    auto tokens = l.lex();

    Parser p(tokens);
    return p.parse();
}


[[nodiscard]] bool is_same(const StmtList& a, const StmtList& b) {
    if (a.size() != b.size()) {
        fmt::print(stderr, "size() is different");
        return false;
    }

    for (int i = 0; i < a.size(); ++i) {
        if (a[i]->to_string() != b[i]->to_string()) {
            fmt::print(stderr, "{} and {} are not equal", a[i].get()->to_string(), b[i].get()->to_string());
            return false;
        }
    }

    return true;
}


TEST(parser, plus) {
    auto got = setup("1+2");

    auto n1 = std::make_unique<Expressions::INumber>("1");
    auto op = Token { .ttype = TokenType::Plus, .lexeme = "+", .position = { .line = 1, .column = 2 } };
    auto n2 = std::make_unique<Expressions::INumber>("2");
    auto expr = std::make_unique<Expressions::BinaryOperator>(std::move(n1), op, std::move(n2));

    StmtList expected;
    expected.push_back(std::make_unique<Statements::ExpressionStatement>(std::move(expr)));
    EXPECT_TRUE(is_same(got, expected));
}

TEST(parser, minus) {
    auto got = setup("1-2");

    auto n1 = std::make_unique<Expressions::INumber>("1");
    auto op = Token { .ttype = TokenType::Minus, .lexeme = "-", .position = { .line = 1, .column = 2 } };
    auto n2 = std::make_unique<Expressions::INumber>("2");
    auto expr = std::make_unique<Expressions::BinaryOperator>(std::move(n1), op, std::move(n2));

    StmtList expected;
    expected.push_back(std::make_unique<Statements::ExpressionStatement>(std::move(expr)));
    EXPECT_TRUE(is_same(got, expected));
}

TEST(parser, minusDnumber) {
    auto got = setup("1-1.5");

    auto n1 = std::make_unique<Expressions::INumber>("1");
    auto op = Token { .ttype = TokenType::Minus, .lexeme = "-", .position = { .line = 1, .column = 2 } };
    auto n2 = std::make_unique<Expressions::DNumber>("1.5");
    auto expr = std::make_unique<Expressions::BinaryOperator>(std::move(n1), op, std::move(n2));

    StmtList expected;
    expected.push_back(std::make_unique<Statements::ExpressionStatement>(std::move(expr)));
    EXPECT_TRUE(is_same(got, expected));
}

TEST(parser, variable) {
    auto got = setup("a");
    auto expr = std::make_unique<Expressions::Variable>("a");
    StmtList expected;
    expected.push_back(std::make_unique<Statements::ExpressionStatement>(std::move(expr)));
    EXPECT_TRUE(is_same(got, expected));
}
