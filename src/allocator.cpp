#include "allocator.hpp"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <algorithm>


ASTAllocator::ASTAllocator(std::size_t blockSize)
: blockSize_(blockSize),
currentBlock_(nullptr),
currentPtr_(nullptr),
blockEnd_(nullptr) {}



ASTAllocator::ASTAllocator(ASTAllocator&& v)
: blockSize_{v.blockSize_},
blocks_{std::move(v.blocks_)},
destructors_{std::move(v.destructors_)},
currentBlock_(v.currentBlock_),
currentPtr_(v.currentPtr_),
blockEnd_(v.blockEnd_) {}


ASTAllocator::~ASTAllocator() {
    destroyAll();
    for (void* block : blocks_) {
        std::free(block);
    }
}


void ASTAllocator::reset() {
    destroyAll();
    destructors_.clear();
    for (void* block : blocks_) {
        std::free(block);
    }
    blocks_.clear();
    currentBlock_ = nullptr;
    currentPtr_ = nullptr;
    blockEnd_ = nullptr;
}

 
void* ASTAllocator::allocate(std::size_t size, std::size_t alignment) {
    assert(alignment && (alignment & (alignment - 1)) == 0 && "Alignment must be power of 2");

    std::size_t space = blockEnd_ - currentPtr_;
    auto curr = reinterpret_cast<std::uintptr_t>(currentPtr_);
    std::uintptr_t aligned = (curr + alignment - 1) & ~(alignment - 1);

    if (std::size_t padding = aligned - curr; padding + size > space) {
        allocateBlock(std::max(blockSize_, size + alignment));
        return allocate(size, alignment);
    }

    auto result = reinterpret_cast<void*>(aligned);
    currentPtr_ = reinterpret_cast<char*>(aligned + size);
    return result;
}


void ASTAllocator::allocateBlock(std::size_t size) {
    void* block = std::malloc(size);
    assert(block && "ASTAllocator block allocation failed");
    blocks_.push_back(block);
    currentBlock_ = static_cast<char*>(block);
    currentPtr_ = currentBlock_;
    blockEnd_ = currentBlock_ + size;
}


void ASTAllocator::destroyAll() {
    for (auto& dtor : destructors_) {
        dtor();
    }
}
