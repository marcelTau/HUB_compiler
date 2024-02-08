#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <spdlog/spdlog.h>
#include <fmt/core.h>

#include "lexer.h"
#include "parser.h"
#include "gen.h"
#include "vm.h"


static auto show_help(void) -> void {
    fmt::print(stderr, R"(
        Usage:
            ./acompiler [file]
    )");
}

static auto readFileToString(const std::string_view path) -> std::optional<std::string> {
    std::filesystem::path filepath { path };

    if (not std::filesystem::exists(filepath)) {
        spdlog::error(fmt::format("File '{}' does not exist.", filepath.string()));
        return std::nullopt;
    }

    std::ifstream file { filepath };
    std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

    return content;
}

auto resolve(std::vector<std::unique_ptr<ByteCode::Instruction>>& instructions) -> void {

    const auto find_label = [&](const auto& name) -> int {
        auto found = std::find_if(std::begin(instructions), std::end(instructions), [name] (auto& inst) {
            if (auto inner = dynamic_cast<ByteCode::Label *>(inst.get())) {
                return inner->label == name;
            }
            return false;
        });

        return std::distance(std::begin(instructions), found);
    };

    for (int i = 0; i < instructions.size(); i++) {
        auto& current = instructions[i];

        if (auto inst = dynamic_cast<ByteCode::Jz *>(current.get())) {
            const auto offset = find_label(inst->label);
            spdlog::info("Jz Found offset: {}", offset);
            inst->offset = offset - i;
        }
        if (auto inst = dynamic_cast<ByteCode::Jmp *>(current.get())) {
            const auto offset = find_label(inst->label);
            spdlog::info("Jmp Found offset: {}", offset);
            inst->offset = offset - i;
        }
    }
}

auto main(int argc, char* argv[]) -> int {
    spdlog::info("Compiler started");

    /*
    if (argc != 2) {
        spdlog::error("No file provided");
        show_help();
        return EXIT_FAILURE;
    }

    const auto content = readFileToString(argv[1]);

    if (not content.has_value()) {
        return EXIT_FAILURE;
    }*/

    auto lexer = Lexer(R"(
                a := 10;
                if a != 10 then
                    print a;
                else
                    print 100;
                end
                print a;
        )"sv);
    auto tokens = lexer.lex();

    fmt::print("=== Tokens ===\n");
    for (const auto& token : tokens) {
        fmt::print(stderr, "{} = {}\n", tokenTypeName.at((int)token.ttype), token);
    }

    auto p = Parser(tokens);

    auto stmts = p.parse();

    fmt::print("=== Statements ===\n");
    for (const auto& stmt : stmts) {
        fmt::print(stderr, "{}\n", stmt->to_string());
    }

    auto generator = BytecodeGenerator(stmts);

    auto outcome = generator.generate();

    resolve(outcome);

    fmt::print("=== Generated ===\n");
    //for (auto value : outcome) {
        //fmt::print(stderr, "0x{:x}\n", value);
    //}

    fmt::print("=== Virtual machine ===\n");
    VirtualMachine vm(outcome);

    vm.execute();

    return EXIT_SUCCESS;
}
