#pragma once
#include <string>
#include <memory>

#include "token.h"

namespace Statements {
    struct Statement;
}

namespace Expressions {
    struct INumber;
    struct DNumber;
    struct BinaryOperator;
    struct Variable; // variable lookup not assign
    struct Logical;
    struct Assign;

    struct ExpressionVisitor {
        virtual void visit(BinaryOperator& expression) = 0;
        virtual void visit(INumber& expression) = 0;
        virtual void visit(DNumber& expression) = 0;
        virtual void visit(Variable& expression) = 0;
        virtual void visit(Logical& expression) = 0;
        virtual void visit(Assign& expression) = 0;
    };

    // ============================================================================
    // Base class of all Expressions
    // ============================================================================
    struct Expression {
        Expression() = default;
        virtual ~Expression() = default;
        virtual void accept(ExpressionVisitor& visitor) = 0;
        [[nodiscard]] virtual auto to_string(std::size_t offset = 0) -> std::string const = 0;
    };

    template<typename T>
    struct ExpressionAcceptor : public Expression {
        void accept(ExpressionVisitor& visitor) final {
            visitor.visit(static_cast<T&>(*this));
        }
    };

    struct BinaryOperator : public ExpressionAcceptor<BinaryOperator> {
        BinaryOperator(std::unique_ptr<Expression> lhs, Token operator_type, std::unique_ptr<Expression> rhs) : lhs{ std::move(lhs) }, operator_type { operator_type }, rhs { std::move(rhs) }{};
        [[nodiscard]] auto to_string(std::size_t offset = 0) -> std::string const final { return "BinaryOperator " + lhs->to_string() + " " + std::string { operator_type.getLexeme() } + " " + rhs->to_string(); };

        std::unique_ptr<Expression> lhs;
        Token operator_type;
        std::unique_ptr<Expression> rhs;
    };

    struct INumber : public ExpressionAcceptor<INumber> {
        INumber(std::string_view sv) {
            value = std::stoi(std::string { sv });
        };
        INumber(INumber&& other) noexcept = default;
        [[nodiscard]] auto to_string(std::size_t offset = 0) -> std::string const final { return "INumber " + std::to_string(value); };

        int value;
    };

    struct DNumber : public ExpressionAcceptor<DNumber> {
        DNumber(std::string_view sv) {
            value = std::stod(std::string { sv });
        };
        DNumber(DNumber&& other) noexcept = default;
        [[nodiscard]] auto to_string(std::size_t offset = 0) -> std::string const final { return "DNumber " + std::to_string(value); };

        double value;
    };

    struct Variable : public ExpressionAcceptor<Variable> {
        Variable(Token name) : name { name } {};
        Variable(Variable&& other) noexcept = default;
        [[nodiscard]] auto to_string(std::size_t offset = 0) -> std::string const final { return "Variable " + std::string { name.getLexeme() }; };

        Token name;
    };

    struct Logical : public ExpressionAcceptor<Logical> {
        Logical(std::unique_ptr<Expression> lhs, Token operator_type, std::unique_ptr<Expression> rhs) : lhs { std::move(lhs) }, operator_type { operator_type }, rhs { std::move(rhs) } {};
        Logical(Logical&& other) noexcept = default;
        [[nodiscard]] auto to_string(std::size_t offset = 0) -> std::string const final { return "Logical " + lhs->to_string() + " " + std::string { operator_type.getLexeme() } + " " + rhs->to_string(); };

        std::unique_ptr<Expression> lhs;
        Token operator_type;
        std::unique_ptr<Expression> rhs;
    };

    struct Assign : public ExpressionAcceptor<Assign> {
        Assign(Token name, std::unique_ptr<Expression> value) : name { name }, value { std::move(value) }{};
        Assign(Assign&& other) noexcept = default;
        [[nodiscard]] auto to_string(std::size_t offset = 0) -> std::string const final { return "Assign " + std::string { name.getLexeme() } + " " + value->to_string(); };

        Token name;
        std::unique_ptr<Expression> value;
    };
}
