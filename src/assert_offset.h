#ifndef ASSERT_OFFSET_H
#define ASSERT_OFFSET_H

#include <cstddef>

// Intentionally incomplete.
template <std::size_t Expected, std::size_t Actual>
struct GotWant;                 

// Only defined for equality.
template <std::size_t X>
struct GotWant<X, X> {
    enum {
        ok = 1,
    };
};

// This checks the offset of a field within a struct/class at compile time.
// First it says what field has mismatching offset, and then it shows the actual vs expected values using the magic in `GotWant<>`.
#define ASSERT_OFFSET(type, field, expected) \
    static_assert(offsetof(type, field) == (expected), #type "::" #field " offset mismatch"); \
    static_assert(GotWant<offsetof(type, field), (expected)>::ok)

// This checks the size of a struct.
// Similarly, uses magic to show actual vs expected size.
#define ASSERT_SIZE(type, expected) \
    static_assert(sizeof(type) == (expected), #type " size mismatch"); \
    static_assert(GotWant<sizeof(type), (expected)>::ok)

#endif
