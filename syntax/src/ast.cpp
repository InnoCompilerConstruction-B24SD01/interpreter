#include "dlang/ast.hpp"

#include <ostream>

namespace dlang {

std::string_view ast_kind_name(AstKind kind) noexcept {
    switch (kind) {
    case AstKind::Program: return "Program";
    case AstKind::Block: return "Block";
    case AstKind::VariableDeclaration: return "VariableDeclaration";
    case AstKind::Variable: return "Variable";
    case AstKind::Assignment: return "Assignment";
    case AstKind::IfStatement: return "IfStatement";
    case AstKind::ShortIfStatement: return "ShortIfStatement";
    case AstKind::WhileLoop: return "WhileLoop";
    case AstKind::ForLoop: return "ForLoop";
    case AstKind::InfiniteLoop: return "InfiniteLoop";
    case AstKind::ExitStatement: return "ExitStatement";
    case AstKind::ReturnStatement: return "ReturnStatement";
    case AstKind::PrintStatement: return "PrintStatement";
    case AstKind::ParameterList: return "ParameterList";
    case AstKind::Parameter: return "Parameter";
    case AstKind::ExpressionBody: return "ExpressionBody";
    case AstKind::BlockBody: return "BlockBody";
    case AstKind::Identifier: return "Identifier";
    case AstKind::IntegerLiteral: return "IntegerLiteral";
    case AstKind::RealLiteral: return "RealLiteral";
    case AstKind::StringLiteral: return "StringLiteral";
    case AstKind::BooleanLiteral: return "BooleanLiteral";
    case AstKind::NoneLiteral: return "NoneLiteral";
    case AstKind::ArrayLiteral: return "ArrayLiteral";
    case AstKind::TupleLiteral: return "TupleLiteral";
    case AstKind::TupleElement: return "TupleElement";
    case AstKind::FunctionLiteral: return "FunctionLiteral";
    case AstKind::UnaryExpression: return "UnaryExpression";
    case AstKind::BinaryExpression: return "BinaryExpression";
    case AstKind::TypeCheckExpression: return "TypeCheckExpression";
    case AstKind::TypeIndicator: return "TypeIndicator";
    case AstKind::IndexExpression: return "IndexExpression";
    case AstKind::CallExpression: return "CallExpression";
    case AstKind::MemberExpression: return "MemberExpression";
    case AstKind::TupleIndexExpression: return "TupleIndexExpression";
    case AstKind::RangeExpression: return "RangeExpression";
    }
    return "Unknown";
}

namespace {

void print_escaped(std::ostream& out, const std::string& value) {
    constexpr char hex[] = "0123456789abcdef";
    out << '"';
    for (const unsigned char ch : value) {
        switch (ch) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (ch < 0x20 || ch == 0x7f) {
                out << "\\u00" << hex[ch >> 4] << hex[ch & 0x0f];
            } else {
                out << static_cast<char>(ch);
            }
        }
    }
    out << '"';
}

} // namespace

void print_ast(std::ostream& out, const AstNode& node, std::size_t indent) {
    for (std::size_t i = 0; i < indent; ++i) {
        out << "  ";
    }
    out << ast_kind_name(node.kind);
    if (!node.value.empty()) {
        out << ' ';
        print_escaped(out, node.value);
    }
    out << '\n';
    for (const auto& child : node.children) {
        print_ast(out, child, indent + 1);
    }
}

std::ostream& operator<<(std::ostream& out, const AstNode& node) {
    print_ast(out, node);
    return out;
}

} // namespace dlang
