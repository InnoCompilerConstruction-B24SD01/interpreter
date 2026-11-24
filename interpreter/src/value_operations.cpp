#include "dlang/value.hpp"

#include <cmath>
#include <limits>
#include <unordered_set>

namespace dlang {
namespace {

using Integer = std::int64_t;
constexpr Integer minInteger = std::numeric_limits<Integer>::min();
constexpr Integer maxInteger = std::numeric_limits<Integer>::max();

bool productOverflows(Integer left, Integer right) {
    if (right > 0) {
        return left > maxInteger / right || left < minInteger / right;
    }
    if (right < -1) {
        return left > minInteger / right || left < maxInteger / right;
    }
    return right == -1 && left == minInteger;
}

Integer arithmetic(const SemanticNode& node, Integer left, Integer right) {
    if (node.value == "+") {
        if ((right > 0 && left > maxInteger - right) ||
            (right < 0 && left < minInteger - right)) {
            throw RuntimeError(node, "integer overflow in addition");
        }
        return left + right;
    }
    if (node.value == "-") {
        if ((right > 0 && left < minInteger + right) ||
            (right < 0 && left > maxInteger + right)) {
            throw RuntimeError(node, "integer overflow in subtraction");
        }
        return left - right;
    }
    if (node.value == "*") {
        if (productOverflows(left, right)) {
            throw RuntimeError(node, "integer overflow in multiplication");
        }
        return left * right;
    }
    if (node.value != "/") {
        throw std::logic_error("unknown arithmetic operator");
    }
    if (right == 0) {
        throw RuntimeError(node, "division by zero");
    }
    if (left == minInteger && right == -1) {
        throw RuntimeError(node, "integer overflow in division");
    }
    const Integer quotient = left / right;
    // C++ truncates toward zero. D rounds a negative, non-integral quotient down.
    if (left % right != 0 && (left < 0) != (right < 0)) {
        return quotient - 1;
    }
    return quotient;
}

double arithmetic(const SemanticNode& node, double left, double right) {
    double result;
    if (node.value == "+") {
        result = left + right;
    } else if (node.value == "-") {
        result = left - right;
    } else if (node.value == "*") {
        result = left * right;
    } else if (node.value == "/") {
        if (right == 0.0) throw RuntimeError(node, "division by zero");
        result = left / right;
    } else {
        throw std::logic_error("unknown arithmetic operator");
    }
    if (!std::isfinite(result)) {
        throw RuntimeError(node, "real overflow");
    }
    return result;
}

template <typename Number>
Value numericOperation(const SemanticNode& node, Number left, Number right) {
    const auto& op = node.value;
    if (op == "<")  return left < right;
    if (op == "<=") return left <= right;
    if (op == ">")  return left > right;
    if (op == ">=") return left >= right;
    if (op == "=")  return left == right;
    if (op == "/=") return left != right;
    return arithmetic(node, left, right);
}

bool isNumber(const Value& value) {
    return std::holds_alternative<Integer>(value) || std::holds_alternative<double>(value);
}

double asReal(const Value& value) {
    if (const auto* integer = std::get_if<Integer>(&value)) {
        return static_cast<double>(*integer);
    }
    return std::get<double>(value);
}

Value concatenate(const Array& left, const Array& right) {
    auto result = std::make_shared<Array>();
    for (const Array* source : {&left, &right}) {
        for (const auto& element : source->elements) {
            const auto index = static_cast<Integer>(result->elements.size()) + 1;
            result->elements.emplace(index, element.second);
        }
    }
    return result;
}

Value concatenate(const SemanticNode& node, const Tuple& left, const Tuple& right) {
    auto result = std::make_shared<Tuple>();
    std::unordered_set<std::string> names;
    for (const Tuple* source : {&left, &right}) {
        for (const auto& element : source->elements) {
            const std::string& name = element.first;
            if (!name.empty() && !names.insert(name).second) {
                throw RuntimeError(node, "duplicate tuple element name '" + name +
                                         "' in concatenation");
            }
            result->elements.push_back(element);
        }
    }
    return result;
}

RuntimeError invalidOperands(const SemanticNode& node, const Value& left, const Value& right) {
    return RuntimeError(node, "operator '" + node.value + "' does not accept " +
                        std::string(valueTypeName(left)) + " and " +
                        std::string(valueTypeName(right)));
}

} // namespace

Value unaryOperation(const SemanticNode& node, const Value& operand) {
    if (node.value == "not") {
        if (const auto* boolean = std::get_if<bool>(&operand)) {
            return !*boolean;
        }
    } else if (const auto* integer = std::get_if<Integer>(&operand)) {
        if (node.value == "+") return *integer;
        if (*integer == minInteger) {
            throw RuntimeError(node, "integer overflow in negation");
        }
        return -*integer;
    } else if (const auto* real = std::get_if<double>(&operand)) {
        return node.value == "+" ? *real : -*real;
    }
    throw RuntimeError(node, "operator '" + node.value + "' does not accept " +
                             std::string(valueTypeName(operand)));
}

Value binaryOperation(const SemanticNode& node, const Value& left, const Value& right) {
    const auto& op = node.value;
    if (op == "and" || op == "or" || op == "xor") {
        const auto* a = std::get_if<bool>(&left);
        const auto* b = std::get_if<bool>(&right);
        if (!a || !b) throw invalidOperands(node, left, right);
        if (op == "and") return *a && *b;
        if (op == "or") return *a || *b;
        return *a != *b;
    }

    if (std::holds_alternative<Integer>(left) && std::holds_alternative<Integer>(right)) {
        return numericOperation(node, std::get<Integer>(left), std::get<Integer>(right));
    }
    if (isNumber(left) && isNumber(right)) {
        return numericOperation(node, asReal(left), asReal(right));
    }
    if (op == "+" && left.index() == right.index()) {
        if (const auto* text = std::get_if<std::string>(&left)) {
            return *text + std::get<std::string>(right);
        }
        if (const auto* array = std::get_if<std::shared_ptr<Array>>(&left)) {
            return concatenate(**array, *std::get<std::shared_ptr<Array>>(right));
        }
        if (const auto* tuple = std::get_if<std::shared_ptr<Tuple>>(&left)) {
            return concatenate(node, **tuple, *std::get<std::shared_ptr<Tuple>>(right));
        }
    }
    throw invalidOperands(node, left, right);
}

} // namespace dlang
