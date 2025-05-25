#pragma once

#include <string>
#include "lox_class.hpp"
#include "string_hash.hpp"

struct LoxInstance: std::enable_shared_from_this<LoxInstance> {
    LoxClass* class_;
    std::unordered_map<std::string, Object, string_hash, std::equal_to<>> fields;

    LoxInstance(LoxClass* class_): class_{class_} {}

    std::string to_string() {
        return this->class_->to_string() + " instance";
    }

    std::expected<Object, InterpreterError> get(const Token& name) {
        if (auto val = this->fields.find(name.lexeme); val != this->fields.end()) {
            return val->second;
        }
        if (auto method = this->class_->find_method(name.lexeme); method != nullptr) {
            return method->bind(this->shared_from_this());
        }
        return std::unexpected(InterpreterError{InterpreterErrorType::UndefinedProperty, name, "Undefined property '" + std::string(name.lexeme) + "'."});
    }

    void set(const Token& name, Object value) {
        auto val = this->fields.find(name.lexeme);
        if (val == this->fields.end()) {
            this->fields[std::string(name.lexeme)] = std::move(value);
        } else {
            val->second = std::move(value);
        }
    }
};
