#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>

#include "expression.h"
#include "statement.h"

namespace ByteCode {
    using Type = std::uint8_t;

    struct Instruction {
        virtual ~Instruction() {}
        const char *const type = "Default";
    };

    struct Print : Instruction {
        const char *const type = "Print";
    };
    struct Add : Instruction {
        const char *const type = "Add";
    };
    struct Sub : Instruction {
        const char *const type = "Sub";
    };
    struct Mul : Instruction {
        const char *const type = "Mul";
    };
    struct Div : Instruction {
        const char *const type = "Div";
    };
    struct Eq : Instruction {
        const char *const type = "Eq";
    };
    struct NEq : Instruction {
        const char *const type = "NEq";
    };
    struct PushInt : Instruction {
        PushInt(int value) : value { value } {}
        const char *const type = "PushInt";
        int value;
    };
    struct PushDouble : Instruction {
        PushDouble(double value) : value { value } {}
        const char *const type = "PushDouble";
        double value;
    };
    struct Assign : Instruction {
        Assign(std::string_view name) : name { name } {}
        std::string_view name;
        const char *const type = "Assign";
    };
    struct Variable : Instruction {
        Variable(std::string_view name) : name { name } {}
        const char *const type = "Variable";
        std::string_view name;
    };
}

class BytecodeGenerator : public Expressions::ExpressionVisitor, Statements::StatementVisitor {
    using Value = std::variant<int, double, std::string>;

public:
  BytecodeGenerator(std::span<std::unique_ptr<Statements::Statement>> statements)
      : statements{ std::move(statements) } {}

  [[nodiscard]] auto generate() -> std::vector<std::unique_ptr<ByteCode::Instruction>> {
      spdlog::warn("=== Start Generating ===");
      for (auto &statement : this->statements) {
          statement->accept(*this);
      }

      return std::move(this->instructions);
  }

private:
    template<typename T>
    auto add_instruction(const T& instruction) -> void {
        spdlog::info(fmt::format("Add instruction {}", instruction.type));
        this->instructions.emplace_back(std::make_unique<T>(instruction));
    }

    // ------------------------------------------------------------------------
    // Statements
    // ------------------------------------------------------------------------
    auto visit(Statements::ExpressionStatement& statement) -> void override {
        statement.expression->accept(*this);
    }

    auto visit(Statements::Print&               statement) -> void override {
        statement.expression->accept(*this);
        add_instruction(ByteCode::Print {});
    }

    // ------------------------------------------------------------------------
    // Expressions
    // ------------------------------------------------------------------------
    auto visit(Expressions::BinaryOperator&     expression) -> void override {
        expression.lhs->accept(*this);
        expression.rhs->accept(*this);

        switch (expression.operator_type.getLexeme()[0]) {
            case '+': return add_instruction(ByteCode::Add {});
            case '-': return add_instruction(ByteCode::Sub {});
            case '*': return add_instruction(ByteCode::Mul {});
            case '/': return add_instruction(ByteCode::Div {});
        };
    }

    auto visit(Expressions::INumber&            expression) -> void override {
        add_instruction(ByteCode::PushInt(expression.value));
    }
    auto visit(Expressions::DNumber&            expression) -> void override {
        add_instruction(ByteCode::PushDouble(expression.value));
    }

    auto visit(Expressions::Variable&            expression) -> void override {
        spdlog::info(fmt::format("Variable expr pushing: {}", expression.name.getLexeme()));
        add_instruction(ByteCode::Variable(expression.name.getLexeme()));
        //add_instruction(ByteCode::Instruction::PUSH_DOUBLE);
        //add_instruction(expression.value);
    }

    auto visit(Expressions::Logical&             expression) -> void override {
        expression.lhs->accept(*this);
        expression.rhs->accept(*this);
        switch (expression.operator_type.ttype) {
            case TokenType::EqualEqual:
                return add_instruction(ByteCode::Eq {});
            case TokenType::BangEqual:
                return add_instruction(ByteCode::NEq {});
            default:
                assert(false);
        }
    }

    auto visit(Expressions::Assign&            expression) -> void override {
        expression.value->accept(*this);
        add_instruction(ByteCode::Assign(expression.name.getLexeme()));
        // OLD
        //if (!variables_index.contains(expression.name.getLexeme())) {
            //variables_index.emplace(expression.name.getLexeme(), variables_index.size());
        //}
        //add_instruction(variables_index.at(expression.name.getLexeme()));
    }
private:
    std::unordered_map<std::string_view, int> variables_index;
    //std::unordered_map<std::string, Value> variables_values;
    std::vector<std::unique_ptr<ByteCode::Instruction>> instructions;
    std::span<std::unique_ptr<Statements::Statement>> statements;
};
