#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>
#include "token.hpp"
#include "tagged_ptr.hpp"


struct ExpressionNode;


constexpr uint64_t ALIGNMENT_REQ = 8;

struct LiteralNode {
    Object value;
};
static_assert(alignof(LiteralNode) == ALIGNMENT_REQ, "LiteralNode does not have correct alignment");


struct UnaryNode {
    Token oper;
    ExpressionNode* operand {};
};
static_assert(alignof(UnaryNode) == ALIGNMENT_REQ, "UnaryNode does not have correct alignment");


struct BinaryNode {
    Token oper;
    ExpressionNode* left {};
    ExpressionNode* right {};
};
static_assert(alignof(BinaryNode) == ALIGNMENT_REQ, "BinaryNode does not have correct alignment");


struct VariableNode {
    Token name;
};
static_assert(alignof(VariableNode) == ALIGNMENT_REQ, "VariableNode does not have correct alignment");


struct AssignmentNode {
    Token name;
    ExpressionNode* expr {};
};
static_assert(alignof(AssignmentNode) == ALIGNMENT_REQ, "AssignmentNode does not have correct alignment");


enum class ExpressionType : uint8_t {
    BINARYOP,
    UNARYOP,
    LITERAL,
    VARIABLE,
    ASSIGNMENT
};


class ExpressionNode {
    TaggedPtr<void, ExpressionType, 0b111> tagged;

public:
    explicit ExpressionNode(BinaryNode* v) { this->set_<BinaryNode>(v); }
    explicit ExpressionNode(UnaryNode* v) { this->set_<UnaryNode>(v); }
    explicit ExpressionNode(LiteralNode* v) { this->set_<LiteralNode>(v); }
    explicit ExpressionNode(VariableNode* v) { this->set_<VariableNode>(v); }
    explicit ExpressionNode(AssignmentNode* v) { this->set_<AssignmentNode>(v); }

    ExpressionType get_type() const { return tagged.get_tag(); }

    BinaryNode* get_binary_node() const { return this->get<BinaryNode>(); }
    UnaryNode* get_unary_node() const { return this->get<UnaryNode>(); }
    LiteralNode* get_literal_node() const { return this->get<LiteralNode>(); }
    VariableNode* get_variable_node() const { return this->get<VariableNode>(); }
    AssignmentNode* get_assignment_node() const { return this->get<AssignmentNode>(); }

    void set(BinaryNode* v) { return this->set_<BinaryNode>(v); }
    void set(UnaryNode* v) { return this->set_<UnaryNode>(v); }
    void set(LiteralNode* v) { return this->set_<LiteralNode>(v); }
    void set(VariableNode* v) { return this->set_<VariableNode>(v); }
    void set(AssignmentNode* v) { return this->set_<AssignmentNode>(v); }

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


template<> constexpr ExpressionType ExpressionNode::get_type_for<BinaryNode>() { return ExpressionType::BINARYOP; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<UnaryNode>() { return ExpressionType::UNARYOP; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<LiteralNode>() { return ExpressionType::LITERAL; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<VariableNode>() { return ExpressionType::VARIABLE; }
template<> constexpr ExpressionType ExpressionNode::get_type_for<AssignmentNode>() { return ExpressionType::ASSIGNMENT; }


class StatementNode;

struct PrintStatementNode {
    ExpressionNode* expr {};
};
static_assert(alignof(PrintStatementNode) == ALIGNMENT_REQ, "PrintStatementNode does not have correct alignment");


struct ExpressionStatementNode {
    ExpressionNode* expr {};
};
static_assert(alignof(ExpressionStatementNode) == ALIGNMENT_REQ, "ExpressionStatementNode does not have correct alignment");


struct VariableDefStatementNode {
    Token name;
    ExpressionNode* initializer {};
};
static_assert(alignof(VariableDefStatementNode) == ALIGNMENT_REQ, "VariableDefStatementNode does not have correct alignment");


struct BlockStatementNode {
    std::vector<StatementNode*> stmts;
};
static_assert(alignof(BlockStatementNode) == ALIGNMENT_REQ, "BlockStatementNode does not have correct alignment");


enum class StatementType {
    PRINT,
    EXPRESSION,
    VARIABLE,
    BLOCK
};


class StatementNode {
    TaggedPtr<void, StatementType, 0b111> tagged;

public:
    explicit StatementNode(PrintStatementNode* v) { this->set_<PrintStatementNode>(v); }
    explicit StatementNode(ExpressionStatementNode* v) { this->set_<ExpressionStatementNode>(v); }
    explicit StatementNode(VariableDefStatementNode* v) { this->set_<VariableDefStatementNode>(v); }
    explicit StatementNode(BlockStatementNode* v) { this->set_<BlockStatementNode>(v); }

    StatementType get_type() const { return tagged.get_tag(); }

    PrintStatementNode* get_print_statement_node() const { return this->get<PrintStatementNode>(); }
    ExpressionStatementNode* get_expression_statement_node() const { return this->get<ExpressionStatementNode>(); }
    VariableDefStatementNode* get_variable_statement_node() const { return this->get<VariableDefStatementNode>(); }
    BlockStatementNode* get_block_statement_node() const { return this->get<BlockStatementNode>(); }

    void set(PrintStatementNode* v) { return this->set_<PrintStatementNode>(v); }
    void set(ExpressionStatementNode* v) { return this->set_<ExpressionStatementNode>(v); }
    void set(VariableDefStatementNode* v) { return this->set_<VariableDefStatementNode>(v); }
    void set(BlockStatementNode* v) { return this->set_<BlockStatementNode>(v); }

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

template<> constexpr StatementType StatementNode::get_type_for<PrintStatementNode>() { return StatementType::PRINT; }
template<> constexpr StatementType StatementNode::get_type_for<ExpressionStatementNode>() { return StatementType::EXPRESSION; }
template<> constexpr StatementType StatementNode::get_type_for<VariableDefStatementNode>() { return StatementType::VARIABLE; }
template<> constexpr StatementType StatementNode::get_type_for<BlockStatementNode>() { return StatementType::BLOCK; }
