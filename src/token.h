#pragma once
#include <string_view>
#include <spdlog/spdlog.h>

enum class TokenType : unsigned int {
    LeftParen,
    RightParen,
    Dot,
    Semicolon,

    // math
    Slash,
    Star,
    Minus,
    Plus,

    Equal,
    EqualEqual,

    INumber,
    DNumber,

    Print,

    Identifier,

    Eof,
};

using namespace std::string_view_literals;

constexpr std::array tokenTypeName = {
    "LeftParen"sv,
    "RightParen"sv,
    "Dot"sv,
    "Semicolon"sv,

    "Slash"sv,
    "Star"sv,
    "Minus"sv,
    "Plus"sv,

    "Equal"sv,
    "EqualEqual"sv,

    "INumber"sv,
    "DNumber"sv,

    "print"sv,

    "identifier"sv,

    "EOF"sv,
};

[[nodiscard]] static constexpr auto getTokenTypeName(const TokenType ttype) -> std::string_view {
    return tokenTypeName.at(static_cast<unsigned int>(ttype));
}

template<>
struct fmt::formatter<TokenType> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx);

    template<typename FormatContext>
    auto format(const TokenType& type, FormatContext& ctx);
};

template<typename ParseContext>
constexpr auto fmt::formatter<TokenType>::parse(ParseContext& ctx) {
    return ctx.begin();
}

template<typename FormatContext>
auto fmt::formatter<TokenType>::format(const TokenType& type, FormatContext& ctx) {
    return fmt::format_to(ctx.out(), "{}", ""/*getTokenTypeName(type)*/);
}


struct TokenPosition {
    unsigned int line;
    unsigned int column;

    [[nodiscard]] constexpr auto operator==(const TokenPosition& other) const noexcept -> bool {
        return line == other.line && column == other.column;
    }
};

struct Token {
    TokenType ttype;
    std::string_view lexeme;
    TokenPosition position { 0, 0 };

    [[nodiscard]] constexpr auto getLexeme() const -> std::string_view { 
        return lexeme; 
    }

    [[nodiscard]] constexpr auto operator==(const Token& other) const -> bool  {
        return this->ttype == other.ttype 
            && this->lexeme == other.lexeme 
            && this->position == other.position;
    }
};

template<>
struct fmt::formatter<Token> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx);

    template<typename FormatContext>
    auto format(const Token& token, FormatContext& ctx) const;
};

template<typename ParseContext>
constexpr auto fmt::formatter<Token>::parse(ParseContext& ctx) {
    return ctx.begin();
}

template<typename FormatContext>
auto fmt::formatter<Token>::format(const Token& token, FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "{} '{}' at [{},{}]", token.ttype, token.lexeme, token.position.line, token.position.column);
}
