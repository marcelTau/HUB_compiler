#pragma once
#include <cctype>
#include <math.h>
#include <optional>
#include <vector>
#include <string_view>
#include <spdlog/spdlog.h>

#include "token.h"

class Lexer {

public:
    using TokenList = std::vector<Token>;

public:
    explicit Lexer(const std::string_view source) : source { source } {
        using namespace std::string_view_literals;
        keywords.insert({ "print"sv, TokenType::Print });
    }

    [[nodiscard]] auto lex(void) noexcept -> TokenList {

        while (this->current < this->source.size()) {
            this->start = this->current;
            nextToken();
        }

        addToken(TokenType::Eof, std::nullopt);

        return this->tokens;
    }

private:
    [[nodiscard]] auto advance(void) -> char {
        this->position.column++;
        return this->source.at(this->current++);
    }

    auto newLine(void) -> void {
        this->position.line++;
        this->position.column = 1;
    }

    [[nodiscard]] auto isAtEnd(std::size_t add = 0) -> bool {
        return this->current + add >= this->source.size();
    }

    [[nodiscard]] auto peek(void) -> char {
        if (this->isAtEnd()) [[unlikely]] {
            spdlog::info("Scanner is at end");
            return '\0';
        }

        return this->source.at(this->current);
    }

    [[nodiscard]] auto peekNext(void) -> char {
        if (this->isAtEnd(1)) [[unlikely]] {
            spdlog::info("Scanner is at end");
            return '\0';
        }

        return this->source.at(this->current + 1);
    }

    [[nodiscard]] auto expect(char expected) -> bool {
        if (this->isAtEnd()) [[unlikely]] {
            return false;
        }

        if (this->source.at(this->current) != expected) {
            return false;
        }

        this->current++;
        return true;
    }

    auto addToken(const TokenType ttype, const std::optional<std::string_view> literal) -> void {
        if (ttype == TokenType::Eof) {
            this->tokens.emplace_back(Token { 
                    .ttype = ttype, 
                    .lexeme = "", 
                    .position = this->position 
            });
        } else {
            const auto lexeme = literal.value_or(std::string_view { 
                    this->source.begin() + this->start, 
                    this->source.begin() + this->current 
            });

            this->tokens.emplace_back(Token { 
                    .ttype = ttype, 
                    .lexeme = lexeme, 
                    .position = this->position 
            });
        }
    }

    [[nodiscard]] auto getCurrentLiteral() -> std::string_view {
        return { 
            this->source.begin() + this->start, 
            this->source.begin() + this->current 
        };
    }

    auto number(void) -> void {
        bool isDouble { false };
        while (std::isdigit(peek())) {
            std::ignore = advance();
        }

        if (peek() == '.' && std::isdigit(peekNext())) {
            isDouble = true;
            std::ignore = advance();
            while (std::isdigit(peek())) {
                std::ignore = advance();
            }
        }

        const auto literal = getCurrentLiteral();

        const auto ttype = isDouble ? TokenType::DNumber : TokenType::INumber;

        addToken(ttype, literal);
    }

    [[nodiscard]] auto isAlpha(char c) -> bool {
        return std::isalpha(c) || c == '_';
    }

    auto identifier(void) -> void {
        while (isAlpha(peek()) || std::isdigit(peek())) {
            std::ignore = advance();
        }

        const auto literal = getCurrentLiteral();

        if (keywords.contains(literal)) {
            addToken(keywords.at(literal), literal);
        } else {
            addToken(TokenType::Identifier, literal);
        }
    }

    auto nextToken(void) noexcept -> void {

#define SINGLE_TOKEN(token_char, token_name) \
        case token_char: addToken(TokenType::token_name, std::nullopt); break;

#define NESTED_TOKEN(first, second, first_name, second_name) \
        case first: {\
            if (expect(second)) {\
                addToken(TokenType::first_name, std::nullopt);\
            } else {\
                addToken(TokenType::second_name, std::nullopt);\
            }\
            break;\
        }\


        const auto c = advance();

        switch (c) {
             SINGLE_TOKEN('(', LeftParen);
             SINGLE_TOKEN(')', RightParen);
             SINGLE_TOKEN('.', Dot);
             SINGLE_TOKEN('-', Minus);
             SINGLE_TOKEN('+', Plus);
             SINGLE_TOKEN(';', Semicolon);
             SINGLE_TOKEN('*', Star);
             SINGLE_TOKEN('/', Slash);
             NESTED_TOKEN('=', '=', EqualEqual, Equal);

            case ' ':
            case '\t':
            case '\r': break;
            case '\n': newLine(); break;

            default:
             if (std::isdigit(c)) {
                 this->number();
             } else if (std::isalpha(c)) {
                 this->identifier();
             } else {
                 spdlog::error(fmt::format("Token not found: {}", c));
             }
        }

#undef SINGLE_TOKEN
#undef NESTED_TOKEN
    }

private:
    // TODO: just make a struct position
    using ScannerPosition = TokenPosition;

    TokenList tokens;
    std::size_t start { 0 };
    std::size_t current { 0 };
    std::string_view source;
    ScannerPosition position { .line = 1, .column = 0 };
    
    std::unordered_map<std::string_view, TokenType> keywords {};
};
