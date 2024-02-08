#include <cstdint>
#include <gtest/gtest.h>
#include "gen.h"
#include "parser.h"
#include "vm.h"

static auto setup(const std::string_view code) -> std::vector<std::unique_ptr<ByteCode::Instruction>> {
    Lexer l(code);
    auto tokens = l.lex();

    Parser p(tokens);
    auto stmts = p.parse();

    BytecodeGenerator g(stmts);

    return g.generate();
}

TEST(gen, print_addition) {
    auto got = setup("print 1 + 2;");

    VirtualMachine vm(got);

    testing::internal::CaptureStdout();
    vm.execute();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "3\n");
}

TEST(gen, print_addition_double) {
    auto got = setup("print 1.5 + 2.2;");

    VirtualMachine vm(got);

    testing::internal::CaptureStdout();
    vm.execute();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "3.7\n");
}

