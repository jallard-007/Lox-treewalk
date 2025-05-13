#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <vector>

class ASTAllocator {
public:
    explicit ASTAllocator(std::size_t blockSize = 4096);
    ~ASTAllocator();

    ASTAllocator(const ASTAllocator&) = delete;
    ASTAllocator& operator=(const ASTAllocator&) = delete;

    ASTAllocator(ASTAllocator&&) noexcept;
    ASTAllocator& operator=(ASTAllocator&&) = delete;

    // Construct and return pointer to new object
    template <typename T, typename... Args>
    T* create(Args&&... args);

    void reset();

    
private:
    const std::size_t blockSize_;
    std::vector<void*> blocks_;
    std::vector<std::function<void()>> destructors_;

    char* currentBlock_;
    char* currentPtr_;
    char* blockEnd_;

    void* allocate(std::size_t size, std::size_t alignment);
    void allocateBlock(std::size_t size);
    void destroyAll();
};


// Inline template for create()
template <typename T, typename... Args>
T* ASTAllocator::create(Args&&... args) {
    void* mem = allocate(sizeof(T), alignof(T));
    T* obj = new (mem) T(std::forward<Args>(args)...);

    if constexpr (!std::is_trivially_destructible_v<T>) {
        destructors_.emplace_back([obj]() {
            obj->~T();
        });
    }

    return obj;
}
