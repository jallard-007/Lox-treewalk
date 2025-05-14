#pragma once

#include <string>
#include <string_view>
#include "token.hpp"
#include "interpreter.hpp"

struct Lox {
    static Interpreter interpreter;
    static bool had_error;
    static bool had_runtime_error;

    static void error(int line, std::string_view message);
    static void error(const Token& token, std::string_view message);
    static void runtime_error(const InterpreterError& error);

    void run(std::string program) const;

    int run_file(const std::string& file) const;

    void run_prompt() const;
};
