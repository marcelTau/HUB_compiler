#pragma once
#include <string>
#include <memory>
#include "expression.h"

namespace Expressions {
    struct Expression;
}

namespace Statements {
    struct ExpressionStatement;
    struct Print;

    struct StatementVisitor {
        virtual void visit(ExpressionStatement& statement) = 0;
        virtual void visit(Print& statement) = 0;
        virtual ~StatementVisitor() = default;
    };

    struct Statement {
        virtual ~Statement() = default;
        virtual void accept(StatementVisitor& visitor) = 0;
        [[nodiscard]] virtual auto to_string(std::size_t offset = 0) -> std::string const = 0;
    };

    template<typename T>
    struct StatementAcceptor : public Statement {
        void accept(StatementVisitor& visitor) final {
            visitor.visit(static_cast<T&>(*this));
        }
    };

    struct ExpressionStatement : public Statements::StatementAcceptor<ExpressionStatement> {
        ExpressionStatement(std::unique_ptr<Expressions::Expression> expr) : expression{std::move(expr)} {};
        [[nodiscard]] auto to_string(std::size_t offset = 0) -> std::string const final { 
            return "ExpressionStatement " + expression->to_string(); 
        };
        std::unique_ptr<Expressions::Expression> expression;
    };

    struct Print : public Statements::StatementAcceptor<Print> {
        Print(std::unique_ptr<Expressions::Expression> expr) : expression{std::move(expr)} {};
        [[nodiscard]] auto to_string(std::size_t offset = 0) -> std::string const final { return "PrintStatement " + expression->to_string(); };

        std::unique_ptr<Expressions::Expression> expression;
    };
}
