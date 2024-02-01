cmake_minimum_required(VERSION 3.22.1)
project(acompiler)
set(CMAKE_CXX_STANDARD 20)

set(TEST_NAME ${PROJECT_NAME}_tests)

include(FetchContent)
FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git GIT_TAG 9.1.0)
FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850)

FetchContent_MakeAvailable(googletest fmt)


set(SOURCES
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} fmt)
target_compile_options(${PROJECT_NAME} PUBLIC)


enable_testing()

add_executable(
    ${TEST_NAME}
    test/lexer.cpp
    test/parser.cpp
    test/gen.cpp
    ${SOURCES}
)

target_include_directories(${TEST_NAME} PUBLIC src)
target_include_directories(${TEST_NAME} PUBLIC spdlog)
target_include_directories(${TEST_NAME} PUBLIC include)

target_link_libraries(
    ${TEST_NAME}
    GTest::gtest_main
    fmt
)

include(GoogleTest)
gtest_discover_tests(${TEST_NAME})
