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
    struct IfStatement;

    struct StatementVisitor {
        virtual void visit(ExpressionStatement& statement) = 0;
        virtual void visit(Print& statement) = 0;
        virtual void visit(IfStatement& statement) = 0;
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

    struct IfStatement : public Statements::StatementAcceptor<IfStatement > {
        IfStatement (std::unique_ptr<Expressions::Expression> condition, 
                        std::unique_ptr<Statements::Statement> then,  
                        std::unique_ptr<Statements::Statement> otherwise) 
            : condition{std::move(condition)} 
            , then { std::move(then) }
            , otherwise { std::move(otherwise) }
        {};
        [[nodiscard]] auto to_string(std::size_t offset = 0) -> std::string const final { 
            return fmt::format("IfStatement {} then {} otherwise", 
                    condition->to_string(), 
                    then->to_string(), 
                    otherwise != nullptr ? otherwise->to_string() : "<no else branch>"); 
        }

        std::unique_ptr<Expressions::Expression> condition;
        std::unique_ptr<Statements::Statement> then;
        std::unique_ptr<Statements::Statement> otherwise;
    };
}
