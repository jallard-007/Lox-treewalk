#include "environment.hpp"
#include <iostream>


void Environment::define(std::string_view n, Object v) {
    this->values.push_back(std::move(v));
    this->values_map[std::string(n)] = this->values.size() - 1;
}


std::expected<Object, InterpreterError> Environment::get(const Token& name) const {
    auto res = this->values_map.find(name.lexeme);
    if (res == this->values_map.end()) {
        return std::unexpected(InterpreterError(InterpreterErrorType::UndefinedVariable, name, "Undefined variable '" + std::string(name.lexeme) + "'."));
    }
    return this->get(res->second);
}

std::expected<Object, InterpreterError> Environment::get(size_t index) const {
    if (index > this->values.size()) {
        return std::unexpected(InterpreterError(InterpreterErrorType::UndefinedVariable, "Undefined variable."));
    }
    return this->values[index];
}


std::expected<Object, InterpreterError> Environment::get_at(int depth, size_t index) const {
    auto env = this->ancestor(depth);
    return env->get(index);
}


Environment* Environment::ancestor(int depth) const {
    const Environment* env = this;
    for (; depth > 0; --depth) {
        if (!env->enclosing) {
            break;
        }
        env = env->enclosing.get();
    }
    return const_cast<Environment*>(env);
}


std::optional<InterpreterError> Environment::assign(const Token& name, Object value) {
    auto res = this->values_map.find(name.lexeme);

    if (res == this->values_map.end()) {
        return InterpreterError(InterpreterErrorType::UndefinedVariable, name, "Undefined variable '" + std::string(name.lexeme) + "'.");
    }

    this->values[res->second] = std::move(value);
    return std::nullopt;
}

std::optional<InterpreterError> Environment::assign(size_t index, Object value) {
    if (index > this->values.size()) {
        return InterpreterError(InterpreterErrorType::UndefinedVariable, "Undefined variable.");
    }
    this->values[index] = std::move(value);
    return std::nullopt;
}


std::optional<InterpreterError> Environment::assign_at(int depth, size_t index, Object value) {
    auto env = this->ancestor(depth);
    return env->assign(index, std::move(value));
}
