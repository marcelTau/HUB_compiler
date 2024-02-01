#pragma once

#include "gen.h"
#include <iostream>
#include <iterator>
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
        EQ,
    };
public:
    VirtualMachine(std::span<ByteCode::Type> bytecode) : bytecode { bytecode }{ ip = std::begin(bytecode); }

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
        case BinaryOperators::EQ:
            if (std::holds_alternative<INumber>(a) && std::holds_alternative<INumber>(b)) {
                stack.emplace_back(std::get<INumber>(a) == std::get<INumber>(b));
            } else if (std::holds_alternative<DNumber>(a) && std::holds_alternative<DNumber>(b)) {
                stack.emplace_back(std::get<DNumber>(a) == std::get<DNumber>(b));
            } else {
                assert(false && "type mismatch");
            }
            break;
        }
    }

    auto execute() -> void {
        const auto readByte = [this]() -> ByteCode::Instruction { 
            return static_cast<ByteCode::Instruction>(*ip++); 
        };

        ByteCode::Instruction instruction;

        while (true && ip != std::end(bytecode)) {
            switch (instruction = readByte()) {
            case ByteCode::Instruction::PRINT:
                std::cout << std::visit(PrintVisitor{}, pop()) << '\n';
                break;
            case ByteCode::Instruction::ADD:
                doBinaryOperation(BinaryOperators::ADD);
                break;
            case ByteCode::Instruction::SUB:
                doBinaryOperation(BinaryOperators::SUB);
                break;
            case ByteCode::Instruction::EQ:
                doBinaryOperation(BinaryOperators::EQ);
                break;
            case ByteCode::Instruction::PUSH_INT:
                stack.push_back(readConstant<int>());
                break;
            case ByteCode::Instruction::PUSH_DOUBLE:
                stack.push_back(readConstant<double>());
                break;
            }
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
    std::span<ByteCode::Type> bytecode;
    std::span<ByteCode::Type>::iterator ip;
    std::vector<Value> stack;
};
