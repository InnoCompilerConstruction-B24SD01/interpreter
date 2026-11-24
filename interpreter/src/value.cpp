#include "dlang/value.hpp"

#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <unordered_set>

namespace dlang {

RuntimeError::RuntimeError(const SemanticNode& node, const std::string& message)
    : std::runtime_error("runtime error at " + std::to_string(node.line) + ':' +
                         std::to_string(node.column) + ": " + message) {}

std::string_view valueTypeName(const Value& value) {
    constexpr std::string_view names[] = {
        "none", "int", "real", "bool", "string", "array", "tuple", "func"
    };
    static_assert(std::size(names) == std::variant_size_v<Value>);
    return names[value.index()];
}

namespace {

std::int64_t parseInteger(const SemanticNode& node, const std::string& text) {
    std::int64_t number{};
    const char* end = text.data() + text.size();
    const auto parsed = std::from_chars(text.data(), end, number);
    if (parsed.ec != std::errc{} || parsed.ptr != end) {
        throw RuntimeError(node, "integer literal is out of range");
    }
    return number;
}

double parseReal(const SemanticNode& node) {
    const char* text = node.value.c_str();
    char* end = nullptr;
    errno = 0;
    const double number = std::strtod(text, &end);
    // Some C libraries report ERANGE for a representable subnormal number.
    if (end != text + node.value.size() || end == text || !std::isfinite(number) ||
        (errno == ERANGE && number == 0.0)) {
        throw RuntimeError(node, "real literal is out of range");
    }
    return number;
}

void appendValue(std::string& output, const Value& value,
                 std::unordered_set<const void*>& active) {
    if (std::holds_alternative<std::monostate>(value)) {
        output += "none";
    } else if (const auto* integer = std::get_if<std::int64_t>(&value)) {
        output += std::to_string(*integer);
    } else if (const auto* real = std::get_if<double>(&value)) {
        char buffer[64];
        const auto result = std::to_chars(std::begin(buffer), std::end(buffer), *real,
                                          std::chars_format::general);
        output.append(buffer, result.ptr);
    } else if (const auto* boolean = std::get_if<bool>(&value)) {
        output += *boolean ? "true" : "false";
    } else if (const auto* text = std::get_if<std::string>(&value)) {
        output += *text;
    } else if (std::holds_alternative<std::shared_ptr<Function>>(value)) {
        output += "<func>";
    } else if (const auto* array = std::get_if<std::shared_ptr<Array>>(&value)) {
        if (!active.insert(array->get()).second) {
            output += "<cycle>";
            return;
        }
        output += '[';
        bool first = true;
        for (const auto& element : (*array)->elements) {
            if (!first) output += ", ";
            appendValue(output, element.second, active);
            first = false;
        }
        output += ']';
        active.erase(array->get());
    } else {
        const auto& tuple = std::get<std::shared_ptr<Tuple>>(value);
        if (!active.insert(tuple.get()).second) {
            output += "<cycle>";
            return;
        }
        output += '{';
        bool first = true;
        for (const auto& [name, element] : tuple->elements) {
            if (!first) output += ", ";
            if (!name.empty()) output += name + " := ";
            appendValue(output, element, active);
            first = false;
        }
        output += '}';
        active.erase(tuple.get());
    }
}

} // namespace

Value literalValue(const SemanticNode& node) {
    switch (node.kind) {
    case AstKind::IntegerLiteral:
        return parseInteger(node, node.value);
    case AstKind::RealLiteral:
        return parseReal(node);
    case AstKind::BooleanLiteral:
        return node.value == "true";
    case AstKind::StringLiteral:
        return node.value.substr(1, node.value.size() - 2);
    case AstKind::NoneLiteral:
        return std::monostate{};
    default:
        throw std::logic_error("expected a scalar literal");
    }
}

Value negatedIntegerLiteral(const SemanticNode& minus) {
    const SemanticNode& literal = minus.children.front();
    return parseInteger(literal, "-" + literal.value);
}

std::string formatValue(const Value& value) {
    std::string result;
    std::unordered_set<const void*> active;
    appendValue(result, value, active);
    return result;
}

} // namespace dlang
