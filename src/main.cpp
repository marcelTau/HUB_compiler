#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <math.h>
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

    auto lexer = Lexer("print 3.7 + 1.5;"sv);
    auto tokens = lexer.lex();

    for (const auto& token : tokens) {
        fmt::print(stderr, "{}\n", token);
    }

    auto p = Parser(tokens);

    auto stmts = p.parse();

    for (const auto& stmt : stmts) {
        fmt::print(stderr, "{}\n", stmt->to_string());
    }

    auto generator = BytecodeGenerator({}, stmts);

    auto outcome = generator.generate();

    for (auto value : outcome) {
        fmt::print(stderr, "0x{:x}\n", value);
    }

    VirtualMachine vm(outcome);

    vm.execute();

    return EXIT_SUCCESS;
}
