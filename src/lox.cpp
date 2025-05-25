
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <optional>
#include "lox.hpp"
#include "scanner.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "resolver.hpp"

std::optional<std::string> read_file_to_string(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);  // binary avoids newline conversion on Windows
    if (!file) {
        return std::nullopt;
    }

    // Read entire file contents into string
    return std::string((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
}


void report(uint32_t line, std::string_view where, std::string_view message) {
    std::cout << "[line " << line << "] Error" << where << ": " << message << '\n';
}


void Lox::error(int line, std::string_view message) {
    Lox::had_error = true;
    report(line, "", message);
}

void Lox::error(const Token& token, std::string_view message) {
    Lox::had_error = true;
    if (token.type == TokenType::END_OF_FILE) {
        report(token.line, " at end", message);
    } else {
        std::string where = " at '";
        where += token.lexeme;
        where += "'";
        report(token.line, where, message);
    }
}

void Lox::runtime_error(const InterpreterError& error) {
    Lox::had_runtime_error = true;
    std::cout << error.msg << "\n[line " << error.where.line << "]\n";
}


void Lox::run(std::string source) const {
    Program program;
    program.source = std::move(source);
    Scanner scanner {program.source, program.tokens};
    program.tokens = scanner.scan();
    Parser parser {program};
    parser.parse();

    // Stop if there was a syntax error.
    if (had_error) return;

    Resolver resolver {Lox::interpreter};
    resolver.resolve(program.statements);

    // Stop if there was a resolution error.
    if (had_error) return;

    Lox::interpreter.interpret(program.statements);
}


int Lox::run_file(const std::string& file) const {
    auto file_content = read_file_to_string(file);
    if (!file_content.has_value()) {
        std::cout << "Could not open file " << file << '\n';
        return 60;
    }
    run(std::move(file_content.value()));

    if (Lox::had_error) return 65;
    if (Lox::had_runtime_error) return 70;
    return 0;
}


void Lox::run_prompt() const {
    Lox::interpreter.repl_mode = true;
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break; // EOF or error
        }
        run(line);
        Lox::had_error = false;
        Lox::had_runtime_error = false;
        std::cout << '\n';
    }
}


int main(int argc, char** argv) {
    if (argc > 2) {
        std::cout << "usage: " << argv[0] << " [script]\n";
        return -1;
    }
    Lox lox {};
    if (argc == 2) {
        return lox.run_file(argv[1]);
    } else {
        lox.run_prompt();
    }
    return 0;
}


bool Lox::had_error = false;
bool Lox::had_runtime_error = false;
Interpreter Lox::interpreter {};
