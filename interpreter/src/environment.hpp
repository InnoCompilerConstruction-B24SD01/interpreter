#pragma once

#include "dlang/value.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace dlang {

class Environment {
public:
    explicit Environment(std::shared_ptr<Environment> parent = {})
        : parent_(std::move(parent)) {}

    Value& declare(std::size_t symbol, Value initial = {}) {
        return cells_.emplace(symbol, std::move(initial)).first->second;
    }

    Value& lookup(std::size_t symbol) {
        for (auto* scope = this; scope; scope = scope->parent_.get()) {
            auto cell = scope->cells_.find(symbol);
            if (cell != scope->cells_.end()) {
                return cell->second;
            }
        }
        throw std::logic_error("analyzed symbol has no runtime binding");
    }

private:
    std::shared_ptr<Environment> parent_;
    std::unordered_map<std::size_t, Value> cells_;
};

struct Function {
    const SemanticNode& definition;
    std::shared_ptr<Environment> closure;
};

} // namespace dlang
