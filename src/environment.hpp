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
    std::unordered_map<std::string, Object, string_hash, std::equal_to<>>values;
public:
    Environment() = default;
    explicit Environment(std::shared_ptr<Environment> enclosing): enclosing{enclosing} {}
    void define(std::string_view, Object);
    std::expected<Object, InterpreterError> get(const Token&) const;
    std::optional<InterpreterError> assign(const Token&, Object);
};
