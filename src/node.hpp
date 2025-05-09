#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include "token.hpp"
#include "tagged_ptr.hpp"


struct ExpressionNode;


#define ALIGNMENT_REQ 8

struct LiteralNode {
    Object value;
};
static_assert(alignof(LiteralNode) == ALIGNMENT_REQ, "LiteralNode does not have correct alignment");


struct UnaryNode {
    Token oper;
    ExpressionNode* operand;
};
static_assert(alignof(UnaryNode) == ALIGNMENT_REQ, "UnaryNode does not have correct alignment");


struct BinaryNode {
    Token oper;
    ExpressionNode* left;
    ExpressionNode* right;
};
static_assert(alignof(BinaryNode) == ALIGNMENT_REQ, "BinaryNode does not have correct alignment");


enum class ExpressionType : uint8_t {
    BinaryOp,
    UnaryOp,
    Literal,
};


class ExpressionNode {
    TaggedPtr<void, ExpressionType, 0b111> tagged;

public:
    ExpressionNode(BinaryNode* v) { this->set_<BinaryNode>(v); }
    ExpressionNode(UnaryNode* v) { this->set_<UnaryNode>(v); }
    ExpressionNode(LiteralNode* v) { this->set_<LiteralNode>(v); }

    ExpressionType get_type() const { return tagged.get_tag(); }

    BinaryNode* get_binary_node() const { return this->get<BinaryNode>(); }
    UnaryNode* get_unary_node() const { return this->get<UnaryNode>(); }
    LiteralNode* get_literal_node() const { return this->get<LiteralNode>(); }

    void set(BinaryNode* v) { return this->set_<BinaryNode>(v); }
    void set(UnaryNode* v) { return this->set_<UnaryNode>(v); }
    void set(LiteralNode* v) { return this->set_<LiteralNode>(v); }

private:
    template<typename T>
    static constexpr ExpressionType get_type_for();

    template<typename T>
    void set_(T* ptr) { tagged.set(ptr, get_type_for<T>()); }

    template<typename T>
    T* get() const {
        assert(get_type_for<T>() == get_type());
        return static_cast<T*>(tagged.get_ptr());
    }
};


template<> constexpr ExpressionType ExpressionNode::get_type_for<BinaryNode>() { return ExpressionType::BinaryOp; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<UnaryNode>() { return ExpressionType::UnaryOp; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<LiteralNode>() { return ExpressionType::Literal; }


struct PrintStatementNode {
    ExpressionNode* expr;
};
static_assert(alignof(PrintStatementNode) == ALIGNMENT_REQ, "PrintStatementNode does not have correct alignment");


struct ExpressionStatementNode {
    ExpressionNode* expr;
};
static_assert(alignof(ExpressionStatementNode) == ALIGNMENT_REQ, "ExpressionStatementNode does not have correct alignment");


enum class StatementType {
    Print,
    Expression
};

class StatementNode {
    TaggedPtr<void, StatementType, 0b111> tagged;

public:
    StatementNode(PrintStatementNode* v) { this->set_<PrintStatementNode>(v); }
    StatementNode(ExpressionStatementNode* v) { this->set_<ExpressionStatementNode>(v); }

    StatementType get_type() const { return tagged.get_tag(); }

    PrintStatementNode* get_print_statement_node() const { return this->get<PrintStatementNode>(); }
    ExpressionStatementNode* get_expression_statement_node() const { return this->get<ExpressionStatementNode>(); }

    void set(PrintStatementNode* v) { return this->set_<PrintStatementNode>(v); }
    void set(ExpressionStatementNode* v) { return this->set_<ExpressionStatementNode>(v); }

private:
    template<typename T>
    static constexpr StatementType get_type_for();

    template<typename T>
    void set_(T* ptr) { tagged.set(ptr, get_type_for<T>()); }

    template<typename T>
    T* get() const {
        assert(get_type_for<T>() == get_type());
        return static_cast<T*>(tagged.get_ptr());
    }
};

template<> constexpr StatementType StatementNode::get_type_for<PrintStatementNode>() { return StatementType::Print; }
template<> constexpr StatementType StatementNode::get_type_for<ExpressionStatementNode>() { return StatementType::Expression; }
