#pragma once

#include "gen.h"
#include <iostream>
#include <iterator>
#include <type_traits>
#include <variant>

using INumber = int;
using DNumber = double;
using Bool = bool;
using Value = std::variant<Bool, INumber, DNumber>;

struct PrintVisitor {
    std::string operator()(Bool b) { return b ? "true" : "false"; }
    // This matches every other type than std::monostate. 
    std::string operator()(const auto& x) { return fmt::format("{}", x); }
};

class VirtualMachine {
    enum class BinaryOperators {
        ADD,
        SUB,
        MUL,
        DIV,
        EQ,
        NEQ,
    };
    static constexpr std::array BinaryOperatorNames = {
        "ADD",
        "SUB",
        "MUL",
        "DIV",
        "EQ",
        "NEQ",
    };


public:
    VirtualMachine(std::span<std::unique_ptr<ByteCode::Instruction>> bytecode) : bytecode { std::move(bytecode) } { ip = std::begin(bytecode); }

    template<typename T>
    auto readConstant(void) -> T {
        T value;
        assert(std::distance(ip, std::end(this->bytecode)) >= sizeof(T));
        std::memcpy(&value, &(*ip), sizeof(T));
        std::advance(ip, sizeof(T));
        return value;
    }

    auto doBinaryOperation(BinaryOperators op) -> void {
        auto b = stack.back();
        stack.pop_back();

        auto a = stack.back();
        stack.pop_back();

        spdlog::info(fmt::format("Perform binary operation {} on {} {}", BinaryOperatorNames[(int) op], std::visit(PrintVisitor{}, a), std::visit(PrintVisitor{}, b)));

        switch (op) {
        case BinaryOperators::ADD:
            if (std::holds_alternative<INumber>(a) && std::holds_alternative<INumber>(b)) {
                stack.emplace_back(std::get<INumber>(a) + std::get<INumber>(b));
            } else if (std::holds_alternative<DNumber>(a) && std::holds_alternative<DNumber>(b)) {
                stack.emplace_back(std::get<DNumber>(a) + std::get<DNumber>(b));
            } else {
                assert(false && "type mismatch");
            }
            break;
        case BinaryOperators::SUB:
            if (std::holds_alternative<INumber>(a) && std::holds_alternative<INumber>(b)) {
                stack.emplace_back(std::get<INumber>(a) - std::get<INumber>(b));
            } else if (std::holds_alternative<DNumber>(a) && std::holds_alternative<DNumber>(b)) {
                stack.emplace_back(std::get<DNumber>(a) - std::get<DNumber>(b));
            } else {
                assert(false && "type mismatch");
            }
            break;
        case BinaryOperators::MUL:
            if (std::holds_alternative<INumber>(a) && std::holds_alternative<INumber>(b)) {
                stack.emplace_back(std::get<INumber>(a) * std::get<INumber>(b));
            } else if (std::holds_alternative<DNumber>(a) && std::holds_alternative<DNumber>(b)) {
                stack.emplace_back(std::get<DNumber>(a) * std::get<DNumber>(b));
            } else {
                assert(false && "type mismatch");
            }
            break;
        case BinaryOperators::DIV:
            if (std::holds_alternative<INumber>(a) && std::holds_alternative<INumber>(b)) {
                stack.emplace_back(std::get<INumber>(a) / std::get<INumber>(b));
            } else if (std::holds_alternative<DNumber>(a) && std::holds_alternative<DNumber>(b)) {
                stack.emplace_back(std::get<DNumber>(a) / std::get<DNumber>(b));
            } else {
                assert(false && "type mismatch");
            }
            break;
        case BinaryOperators::EQ:
            if (std::holds_alternative<INumber>(a) && std::holds_alternative<INumber>(b)) {
                stack.emplace_back(std::get<INumber>(a) == std::get<INumber>(b));
            } else if (std::holds_alternative<DNumber>(a) && std::holds_alternative<DNumber>(b)) {
                stack.emplace_back(std::get<DNumber>(a) == std::get<DNumber>(b));
            } else {
                assert(false && "type mismatch");
            }
            break;
        case BinaryOperators::NEQ:
            if (std::holds_alternative<INumber>(a) && std::holds_alternative<INumber>(b)) {
                stack.emplace_back(std::get<INumber>(a) != std::get<INumber>(b));
            } else if (std::holds_alternative<DNumber>(a) && std::holds_alternative<DNumber>(b)) {
                stack.emplace_back(std::get<DNumber>(a) != std::get<DNumber>(b));
            } else {
                assert(false && "type mismatch");
            }
            break;
        }
    }

    auto execute() -> void {
        const auto readInstruction = [this]() -> std::unique_ptr<ByteCode::Instruction> { 
            return std::move(*ip++);
        };

        ByteCode::Instruction instruction;

        spdlog::warn("=== Start VM ===");

        while (true && ip != std::end(bytecode)) {
            const auto inst = readInstruction();

            if (dynamic_cast<ByteCode::Print *>(inst.get())) {
                std::cout << std::visit(PrintVisitor{}, pop()) << '\n';
            } if (dynamic_cast<ByteCode::Add *>(inst.get())) {
                doBinaryOperation(BinaryOperators::ADD);
            } else if (dynamic_cast<ByteCode::Sub *>(inst.get())) {
                doBinaryOperation(BinaryOperators::SUB);
            } else if (dynamic_cast<ByteCode::Mul *>(inst.get())) {
                doBinaryOperation(BinaryOperators::MUL);
            } else if (dynamic_cast<ByteCode::Div *>(inst.get())) {
                doBinaryOperation(BinaryOperators::DIV);
            } else if (dynamic_cast<ByteCode::Eq *>(inst.get())) {
                doBinaryOperation(BinaryOperators::EQ);
            } else if (dynamic_cast<ByteCode::NEq *>(inst.get())) {
                doBinaryOperation(BinaryOperators::NEQ);
            } else if (dynamic_cast<ByteCode::PushInt *>(inst.get())) {
                const auto value = dynamic_cast<ByteCode::PushInt *>(inst.get())->value;
                spdlog::info(fmt::format("PushInt [{}]", value));
                this->stack.push_back(value);
            } else if (dynamic_cast<ByteCode::PushDouble *>(inst.get())) {
                assert(false && "Not implemented");
            } else if (dynamic_cast<ByteCode::Assign *>(inst.get())) {
                const auto value = pop();
                const auto name = dynamic_cast<ByteCode::Assign *>(inst.get())->name;
                spdlog::info(fmt::format("Assign [{}] to {}", std::visit(PrintVisitor{}, value), name));
                this->variables.insert({ name, value });
            } else if (dynamic_cast<ByteCode::Variable *>(inst.get())) {
                const auto name = dynamic_cast<ByteCode::Variable *>(inst.get())->name;
                spdlog::info(fmt::format("Lookup variable {}", name));
                if (not this->variables.contains(name)) {
                    fmt::print(stderr, "No variable with name '{}'", name);
                    assert(false);
                }

                this->stack.push_back(this->variables.at(name));
            }

            //perform(inst);


            //switch (instruction = readInstruction()) {
            //case ByteCode::Instruction::PRINT:
                //std::cout << std::visit(PrintVisitor{}, pop()) << '\n';
                //break;
            //case ByteCode::Instruction::ADD:
                //doBinaryOperation(BinaryOperators::ADD);
                //break;
            //case ByteCode::Instruction::SUB:
                //doBinaryOperation(BinaryOperators::SUB);
                //break;
            //case ByteCode::Instruction::EQ:
                //doBinaryOperation(BinaryOperators::EQ);
                //break;
            //case ByteCode::Instruction::NEQ:
                //doBinaryOperation(BinaryOperators::NEQ);
                //break;
            //case ByteCode::Instruction::PUSH_INT:
                //stack.push_back(readConstant<int>());
                //break;
            //case ByteCode::Instruction::PUSH_DOUBLE:
                //stack.push_back(readConstant<double>());
                //break;
            //}
        }

        //fmt::print("end stack\n");
        //for (auto dump = stack; not dump.empty(); dump.pop_back()) {
            //fmt::print("[ {} ]", std::visit(PrintVisitor{}, dump.back()));
        //}
        //fmt::print("\n");
    }
    
private:

    [[nodiscard]] auto pop() -> Value {
        auto v = stack.back();
        stack.pop_back();
        return v;
    }

private:
    std::span<std::unique_ptr<ByteCode::Instruction>> bytecode;
    std::span<std::unique_ptr<ByteCode::Instruction>>::iterator ip;
    std::vector<Value> stack;
    std::unordered_map<std::string_view, Value> variables;
};
