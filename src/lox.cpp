
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include "lox.hpp"
#include "scanner.hpp"
#include "parser.hpp"
#include "interpreter.hpp"


std::string read_file_to_string(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);  // binary avoids newline conversion on Windows
    if (!file) {
        throw std::runtime_error("Could not open file: " + std::string(filename));
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
    std::cout << error.msg << "\n[line " << error.where.line << "]\n";
    Lox::had_runtime_error = true;
}


void Lox::run(std::string program) {
    Scanner scanner{program};
    auto tokens = scanner.scan();
    ASTAllocator allocator = ASTAllocator();
    Parser parser = Parser(std::move(allocator), tokens);
    auto expr = parser.parse();

    // Stop if there was a syntax error.
    if (had_error) return;
    if (!expr.has_value()) return;

    Lox::interpreter.interpret(expr.value());
    // for (auto& token : tokens) {
    //     std::cout << token;
    // }
}


int Lox::run_file(const std::string& file) {
    auto file_content = read_file_to_string(file);
    run(file_content);

    if (Lox::had_error) return 65;
    if (Lox::had_runtime_error) return 70;
    return 0;
}


void Lox::run_prompt() {
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break; // EOF or error
        }
        run(std::move(line));
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
    Lox lox = Lox();
    if (argc == 2) {
        return lox.run_file(argv[1]);
    } else {
        lox.run_prompt();
    }
    return 0;
}


bool Lox::had_error = false;
bool Lox::had_runtime_error = false;
Interpreter Lox::interpreter = Interpreter();
