#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace dlang {

struct Array;
struct Tuple;
struct Function;

class RuntimeObjects {
public:
    RuntimeObjects() = default;
    RuntimeObjects(const RuntimeObjects&) = delete;
    RuntimeObjects& operator=(const RuntimeObjects&) = delete;
    ~RuntimeObjects();

    void track(const std::shared_ptr<Array>& array);
    void track(const std::shared_ptr<Tuple>& tuple);
    void track(const std::shared_ptr<Function>& function);

private:
    std::vector<std::weak_ptr<Array>> arrays_;
    std::vector<std::weak_ptr<Tuple>> tuples_;
    std::vector<std::weak_ptr<Function>> functions_;
    std::size_t nextSweep_ = 64;

    void pruneExpired();
};

} // namespace dlang
