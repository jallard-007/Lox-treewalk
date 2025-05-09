#pragma once

#include <unordered_map>
#include <expected>
#include <optional>
#include <functional>
#include <string>
#include <string_view>
#include "token.hpp"
#include "errors.hpp"


struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
 
    std::size_t operator()(const char* str) const        { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
};


class Environment {
    std::unordered_map<std::string, Object, string_hash, std::equal_to<>>values;
public:
    void define(std::string_view, const Object&);
    std::expected<Object, InterpreterError> get(const Token&);
    std::optional<InterpreterError> assign(const Token&, const Object&);
};
