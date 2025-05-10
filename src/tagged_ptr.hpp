#pragma once

#include <compare>
#include <cassert>
#include <cstdint>

template<typename T, typename TagEnum, uint64_t TagMask>
class TaggedPtr {
    uintptr_t raw;

public:
    TaggedPtr() : raw(0) {}
    TaggedPtr(T* ptr, TagEnum tag): raw{(uintptr_t)ptr | (uintptr_t)tag} {
        assert(((uintptr_t)ptr & TagMask) == 0);
    }

    T* get_ptr() const {
        return reinterpret_cast<T*>(raw & ~TagMask);
    }

    TagEnum get_tag() const {
        return static_cast<TagEnum>(raw & TagMask);
    }

    void set(T* ptr, TagEnum tag) {
        assert(((uintptr_t)ptr & TagMask) == 0);
        raw = (uintptr_t)ptr | (uintptr_t)tag;
    }

    bool operator<=>(const TaggedPtr& other) const = default;
};
