// Public interface — see plan/DECISIONS.md D-020. Stub stage: every function throws cs::NotImplemented.
#pragma once
#include <stdexcept>
namespace cs {
struct NotImplemented : std::logic_error { using std::logic_error::logic_error; };
}
