#pragma once

#include "dlang/semantic.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace dlang {

struct Array;
struct Tuple;
struct Function;

using Value = std::variant<std::monostate, std::int64_t, double, bool, std::string,
                           std::shared_ptr<Array>, std::shared_ptr<Tuple>,
                           std::shared_ptr<Function>>;

struct Array {
    std::map<std::int64_t, Value> elements;
};

struct Tuple {
    std::vector<std::pair<std::string, Value>> elements;
};

class RuntimeError : public std::runtime_error {
public:
    RuntimeError(const SemanticNode& node, const std::string& message);
};

std::string_view valueTypeName(const Value& value);
std::string formatValue(const Value& value);

Value literalValue(const SemanticNode& node);
Value negatedIntegerLiteral(const SemanticNode& minus);
Value unaryOperation(const SemanticNode& node, const Value& operand);
Value binaryOperation(const SemanticNode& node, const Value& left, const Value& right);

} // namespace dlang
