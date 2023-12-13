#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>

#include "expression.h"
#include "statement.h"

namespace ByteCode {
    using Type = std::uint8_t;

    enum class Instruction {
        PRINT,

        ADD,
        SUB,

        PUSH_INT,
        PUSH_DOUBLE,
    };
}

class BytecodeGenerator : public Expressions::ExpressionVisitor, Statements::StatementVisitor {
    using Value = std::variant<int, std::string>;

public:
  BytecodeGenerator(std::unordered_map<std::string, Value> variables,
                    std::span<std::unique_ptr<Statements::Statement>> statements)
      : variables{ std::move(variables) }, statements{ std::move(statements) } {}

  [[nodiscard]] auto generate() -> std::vector<ByteCode::Type> {
      for (auto &statement : this->statements) {
          statement->accept(*this);
      }

      return this->instructions;
  }

private:
  const std::unordered_map<std::string_view, ByteCode::Instruction> BinaryOperatorMap = {
      { "+", ByteCode::Instruction::ADD },
      { "-", ByteCode::Instruction::SUB },
  };

private:
    auto add_instruction(ByteCode::Instruction instruction) -> void {
        this->instructions.push_back(static_cast<ByteCode::Type>(instruction));
    }

    template<typename T>
    auto add_instruction(T value) -> void {
        uint8_t byteArray[sizeof(T)];
        std::memcpy(byteArray, &value, sizeof(T));
        this->instructions.insert(std::end(this->instructions), std::begin(byteArray), std::end(byteArray));
    }

    // ------------------------------------------------------------------------
    // Statements
    // ------------------------------------------------------------------------
    auto visit(Statements::ExpressionStatement& statement) -> void override {
        statement.expression->accept(*this);
    }

    auto visit(Statements::Print&               statement) -> void override {
        statement.expression->accept(*this);
        add_instruction(ByteCode::Instruction::PRINT);
    }

    // ------------------------------------------------------------------------
    // Expressions
    // ------------------------------------------------------------------------
    auto visit(Expressions::BinaryOperator&     expression) -> void override {
        expression.lhs->accept(*this);
        expression.rhs->accept(*this);

        add_instruction(BinaryOperatorMap.at(expression.operator_type.getLexeme()));
    }

    auto visit(Expressions::INumber&            expression) -> void override {
        add_instruction(ByteCode::Instruction::PUSH_INT);
        add_instruction(expression.value);
    }
    auto visit(Expressions::DNumber&            expression) -> void override {
        add_instruction(ByteCode::Instruction::PUSH_DOUBLE);
        add_instruction(expression.value);
    }

private:
    std::unordered_map<std::string, Value> variables;
    std::vector<ByteCode::Type> instructions;
    std::span<std::unique_ptr<Statements::Statement>> statements;
};
