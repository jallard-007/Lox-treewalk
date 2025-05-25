#pragma once

#include <unordered_map>
#include <expected>
#include <optional>
#include <functional>
#include <string>
#include <string_view>
#include "token.hpp"
#include "errors.hpp"
#include "string_hash.hpp"


class Environment {
    std::shared_ptr<Environment> enclosing;
    std::unordered_map<std::string, size_t, string_hash, std::equal_to<>> values_map;
    std::vector<Object> values;
public:
    Environment() = default;
    explicit Environment(std::shared_ptr<Environment> enclosing): enclosing{enclosing} {}
    void define(std::string_view, Object);
    Environment* ancestor(int) const;
    std::expected<Object, InterpreterError> get(const Token&) const;
    std::expected<Object, InterpreterError> get(size_t) const;
    std::expected<Object, InterpreterError> get_at(int, size_t) const;
    std::optional<InterpreterError> assign(const Token&, Object);
    std::optional<InterpreterError> assign(size_t, Object);
    std::optional<InterpreterError> assign_at(int, size_t, Object);
};
