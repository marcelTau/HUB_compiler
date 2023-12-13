#pragma once
#include "expression.h"
#include "lexer.h"
#include "statement.h"

class Parser {
    using TokenList = Lexer::TokenList;

    using UniqStmt = std::unique_ptr<Statements::Statement>;
    using UniqExpr = std::unique_ptr<Expressions::Expression>;

    using StatementList = std::vector<UniqStmt>;
public:
    Parser(const std::span<Token> tokens) : tokens{ tokens } {}

    [[nodiscard]] auto parse() -> StatementList {
        StatementList statements;
        while (! isAtEnd()) {
            auto decl = declaration();
            statements.push_back(std::move(decl));
        }

        return statements;
    }

    [[nodiscard]] auto declaration() -> UniqStmt {
        return statement();
    }


    [[nodiscard]] auto statement() -> UniqStmt {
        if (checkAndAdvance(TokenType::Print)) {
            return printStatement();
        }

        return expressionStatement();
    }


    [[nodiscard]] auto expressionStatement() -> UniqStmt {
        auto expr = expression();
        std::ignore = consume(TokenType::Semicolon, "Expect ';' after expression.");
        auto exprStatement = std::make_unique<Statements::ExpressionStatement>(std::move(expr));
        return UniqStmt(std::move(exprStatement));
    }

    
    [[nodiscard]] auto printStatement() -> UniqStmt {
        auto expr = expression();
        std::ignore = consume(TokenType::Semicolon, "Expect ';' after value.");

        auto printExpr = std::make_unique<Statements::Print>(std::move(expr));
        return UniqStmt(std::move(printExpr));
    }


    // Expressions

    auto expression() -> UniqExpr {
        return assignment();
    }


    auto assignment() -> UniqExpr {
        auto expr = term();
        return expr;
    }


    auto term() -> UniqExpr {
        auto expr = factor();


        while (checkAndAdvance(TokenType::Plus, TokenType::Minus)) {
            auto op = previous();
            auto right = factor();
            expr = std::make_unique<Expressions::BinaryOperator>(std::move(expr), op, std::move(right));
        }

        return expr;
    }


    auto factor() -> UniqExpr {
        auto expr = primary();

        while (checkAndAdvance(TokenType::Star, TokenType::Slash)) {
            auto op = previous();
            auto right = primary();
            expr = std::make_unique<Expressions::BinaryOperator>(std::move(expr), op, std::move(right));
        }

        return expr;
    }


    auto primary() -> UniqExpr {
        if (checkAndAdvance(TokenType::INumber)) {
            auto number = std::make_unique<Expressions::INumber>(previous().lexeme);
            return UniqExpr(std::move(number));
        }
        if (checkAndAdvance(TokenType::DNumber)) {
            auto number = std::make_unique<Expressions::DNumber>(previous().lexeme);
            return UniqExpr(std::move(number));
        }

        assert(false && "primary failed");
    }



    template <typename ...Tokens>
    [[nodiscard]] auto checkAndAdvance(Tokens&& ...tokens) -> bool {
        const bool found = (check(tokens) || ...);
        if (found) {
            std::ignore = advance();
        }
        return found;
    }


    [[nodiscard]] constexpr auto isAtEnd() const -> bool {
        return peek().ttype == TokenType::Eof;
    }

    [[nodiscard]] constexpr auto peek() const -> Token {
        return this->tokens[this->current];
    }

    [[nodiscard]] auto consume(const TokenType& ttype, const std::string& msg) -> Token {
        if (check(ttype)) {
            return advance();
        } else {
            return /* some error (peek(), msg)*/ {};
        }
    }

    [[nodiscard]] auto check(const TokenType& ttype) const -> bool {
        if (isAtEnd()) {
            return false;
        }
        return peek().ttype == ttype;
    }

    [[nodiscard]] auto advance() -> Token {
        if (not isAtEnd()) {
            this->current++;
        }
        return previous();
    }

    [[nodiscard]] auto previous() const -> Token {
        return this->tokens[this->current - 1];
    }

private:
    std::size_t current { 0 };
    std::span<Token> tokens;
};
