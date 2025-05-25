#include "lox_class.hpp"
#include "lox_instance.hpp"

LoxClass::LoxClass(std::string_view name, std::unordered_map<std::string, std::shared_ptr<LoxFunction>, string_hash, std::equal_to<>> methods): name{name}, methods{std::move(methods)} {}

std::string LoxClass::to_string() {
    return std::string(this->name);
}

std::optional<InterpreterSignal> LoxClass::call(Interpreter& interpreter, std::vector<Object>& arguments) {
    auto inst = std::make_shared<LoxInstance>(this);
    auto x = this->find_method("init");
    if (x) {
        return x->bind(inst)->call(interpreter, arguments);
    }
    return ReturnSignal{inst};
}

size_t LoxClass::arity() {
    auto x = this->find_method("init");
    if (x) {
        return x->arity();
    }
    return 0;
}

std::shared_ptr<LoxFunction> LoxClass::find_method(std::string_view name) {
    auto method = this->methods.find(name);
    if (method == this->methods.end()) {
        return nullptr;
    }
    return method->second;
}
