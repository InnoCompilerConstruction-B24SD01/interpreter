#include "execution.hpp"

#include <charconv>
#include <stdexcept>
#include <utility>

namespace dlang {
namespace {

Value& tupleElement(Tuple& tuple, const SemanticNode& access) {
    if (access.kind == AstKind::MemberExpression) {
        for (auto& [name, value] : tuple.elements) {
            if (name == access.value) {
                return value;
            }
        }
        throw RuntimeError(access, "tuple has no field '" + access.value + "'");
    }

    std::size_t position = 0;
    const char* end = access.value.data() + access.value.size();
    auto parsed = std::from_chars(access.value.data(), end, position);
    if (parsed.ec != std::errc{} || position == 0 || position > tuple.elements.size()) {
        throw RuntimeError(access, "tuple position " + access.value + " is out of range");
    }
    return tuple.elements[position - 1].second;
}

} // namespace

Value Execution::evaluateExpression(const SemanticNode& node, const Scope& scope) {
    switch (node.kind) {
    case AstKind::Identifier:
        return scope->lookup(node.symbol_id);
    case AstKind::NoneLiteral:
    case AstKind::IntegerLiteral:
    case AstKind::RealLiteral:
    case AstKind::StringLiteral:
    case AstKind::BooleanLiteral:
        return literalValue(node);
    case AstKind::UnaryExpression: {
        const auto& operand = node.children[0];
        // The magnitude of the smallest signed integer does not fit in an int64.
        if (node.value == "-" && operand.kind == AstKind::IntegerLiteral &&
            operand.value.front() != '-') {
            return negatedIntegerLiteral(node);
        }
        return unaryOperation(node, evaluateExpression(operand, scope));
    }
    case AstKind::BinaryExpression: {
        const Value left = evaluateExpression(node.children[0], scope);
        const Value right = evaluateExpression(node.children[1], scope);
        Value result = binaryOperation(node, left, right);
        if (auto array = std::get_if<std::shared_ptr<Array>>(&result)) {
            objects_.track(*array);
        } else if (auto tuple = std::get_if<std::shared_ptr<Tuple>>(&result)) {
            objects_.track(*tuple);
        }
        return result;
    }
    case AstKind::TypeCheckExpression:
        return valueTypeName(evaluateExpression(node.children[0], scope)) ==
               node.children[1].value;
    case AstKind::ArrayLiteral: {
        auto array = std::make_shared<Array>();
        objects_.track(array);
        for (const auto& element : node.children) {
            const auto key = static_cast<std::int64_t>(array->elements.size()) + 1;
            array->elements.emplace(key, evaluateExpression(element, scope));
        }
        return array;
    }
    case AstKind::TupleLiteral: {
        auto tuple = std::make_shared<Tuple>();
        objects_.track(tuple);
        for (const auto& field : node.children) {
            tuple->elements.emplace_back(field.value, evaluateExpression(field.children[0], scope));
        }
        return tuple;
    }
    case AstKind::IndexExpression: {
        auto [array, key] = arrayLocation(node, scope);
        auto found = array->elements.find(key);
        if (found == array->elements.end()) {
            throw RuntimeError(node, "array has no key " + std::to_string(key));
        }
        return found->second;
    }
    case AstKind::MemberExpression:
    case AstKind::TupleIndexExpression: {
        auto tuple = requireType<std::shared_ptr<Tuple>>(evaluateExpression(node.children[0], scope),
                                                         node, "field access requires tuple");
        return tupleElement(*tuple, node);
    }
    case AstKind::FunctionLiteral: {
        auto function = std::make_shared<Function>(Function{node, scope});
        objects_.track(function);
        return function;
    }
    case AstKind::CallExpression:
        return callFunction(node, scope);
    default:
        throw std::logic_error("expected an expression in the analyzed tree");
    }
}

Execution::ArrayLocation Execution::arrayLocation(const SemanticNode& node, const Scope& scope) {
    auto array = requireType<std::shared_ptr<Array>>(evaluateExpression(node.children[0], scope),
                                                     node, "indexing requires array");
    auto key = requireType<std::int64_t>(evaluateExpression(node.children[1], scope),
                                         node, "array index must be int");
    return {std::move(array), key};
}

void Execution::assign(const SemanticNode& target, const SemanticNode& expression,
                       const Scope& scope) {
    switch (target.kind) {
    case AstKind::Identifier:
        scope->lookup(target.symbol_id) = evaluateExpression(expression, scope);
        return;
    case AstKind::IndexExpression: {
        auto [array, key] = arrayLocation(target, scope);
        Value value = evaluateExpression(expression, scope);
        array->elements.insert_or_assign(key, std::move(value));
        return;
    }
    case AstKind::MemberExpression:
    case AstKind::TupleIndexExpression: {
        auto tuple = requireType<std::shared_ptr<Tuple>>(evaluateExpression(target.children[0], scope),
                                                         target, "field access requires tuple");
        Value& field = tupleElement(*tuple, target);
        field = evaluateExpression(expression, scope);
        return;
    }
    default:
        throw RuntimeError(target, "invalid assignment target");
    }
}

Value Execution::callFunction(const SemanticNode& call, const Scope& caller) {
    if (callDepth_ >= maxCallDepth) {
        throw RuntimeError(call, "maximum call depth exceeded (" +
                                 std::to_string(maxCallDepth) + ")");
    }
    struct LeaveCall {
        unsigned& depth;
        ~LeaveCall() { --depth; }
    };
    ++callDepth_;
    const LeaveCall guard{callDepth_};

    auto function = requireType<std::shared_ptr<Function>>(
        evaluateExpression(call.children[0], caller), call, "call requires func");
    const auto& parameters = function->definition.children[0].children;
    const auto& body = function->definition.children[1];
    const auto argumentCount = call.children.size() - 1;
    if (parameters.size() != argumentCount) {
        throw RuntimeError(call, "expected " + std::to_string(parameters.size()) +
                                 " argument(s), got " + std::to_string(argumentCount));
    }

    auto locals = std::make_shared<Environment>(function->closure);
    for (std::size_t i = 0; i < parameters.size(); ++i) {
        locals->declare(parameters[i].symbol_id, evaluateExpression(call.children[i + 1], caller));
    }
    if (body.kind == AstKind::ExpressionBody) {
        return evaluateExpression(body.children[0], locals);
    }
    return executeBlock(body.children[0], locals).value;
}

} // namespace dlang
