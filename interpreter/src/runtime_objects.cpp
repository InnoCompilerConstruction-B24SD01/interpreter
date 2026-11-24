#include "runtime_objects.hpp"
#include "environment.hpp"

#include <algorithm>

namespace dlang {
namespace {

template <typename T>
void eraseExpired(std::vector<std::weak_ptr<T>>& objects) {
    const auto expired = [](const auto& object) { return object.expired(); };
    objects.erase(std::remove_if(objects.begin(), objects.end(), expired), objects.end());
}

} // namespace

void RuntimeObjects::track(const std::shared_ptr<Array>& array) {
    arrays_.push_back(array);
    pruneExpired();
}

void RuntimeObjects::track(const std::shared_ptr<Tuple>& tuple) {
    tuples_.push_back(tuple);
    pruneExpired();
}

void RuntimeObjects::track(const std::shared_ptr<Function>& function) {
    functions_.push_back(function);
    pruneExpired();
}

void RuntimeObjects::pruneExpired() {
    if (arrays_.size() + tuples_.size() + functions_.size() < nextSweep_) {
        return;
    }
    eraseExpired(arrays_);
    eraseExpired(tuples_);
    eraseExpired(functions_);
    const auto live = arrays_.size() + tuples_.size() + functions_.size();
    nextSweep_ = 64 + live * 2;
}

RuntimeObjects::~RuntimeObjects() {
    // A recursive function can own its own scope, and a collection can contain itself.
    for (const auto& entry : functions_) {
        if (auto function = entry.lock()) {
            function->closure.reset();
        }
    }
    for (const auto& entry : arrays_) {
        if (auto array = entry.lock()) {
            array->elements.clear();
        }
    }
    for (const auto& entry : tuples_) {
        if (auto tuple = entry.lock()) {
            tuple->elements.clear();
        }
    }
}

} // namespace dlang
