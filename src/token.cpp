#include "token.hpp"

std::ostream& operator<<(std::ostream& os, const Token& t) {
    return os << "[" << t.line << "] " << t.lexeme << " -> [type: " << t.type << "] " << t.val;
}


std::ostream& operator<<(std::ostream& os, const None&) {
    return os << "None";
}


std::ostream& operator<<(std::ostream& os, const Object& v) {
    std::visit([&os](auto&& arg) {
        os << arg;  // dispatches to correct overload
    }, v);
    return os;
}

std::ostream& operator<<(std::ostream& os, const TokenType& v) {
    if (static_cast<size_t>(v) >= token_map.size()) {
        os << "Unknown token enum value";
    } else {
        os << token_map[static_cast<size_t>(v)];
    }
    return os;
}