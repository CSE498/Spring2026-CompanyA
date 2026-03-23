/**
 * @brief A memory manager class that pre-allocates blocks of memory for
 *  a specific type T and manages a free list for efficient allocation and
 *deallocation.
 * @note Status: PROPOSAL
 **/

// Refactored this class to use smart pointers using an LLM

#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <ranges>
#include <utility>
#include <vector>

namespace cse498 {

// Templated to work for any type T
template <class T> class MemoryFactory {

public:
  explicit MemoryFactory(std::size_t blockSize = 64)
      : blockSize(blockSize), freeList(nullptr) {
    assert(blockSize > 0);
    allocateNewBlock();
  }

  MemoryFactory(const MemoryFactory &) = delete;
  MemoryFactory &operator=(const MemoryFactory &) = delete;

  // Destructor to free all allocated blocks of memory
  ~MemoryFactory() {
    std::ranges::for_each(blocks, [](void *block) {
      ::operator delete(block, std::align_val_t(alignof(Node)));
    });
  }

  struct PoolDeleter {
    MemoryFactory *pool{nullptr};

    void operator()(T *ptr) const noexcept {
      if (pool && ptr) {
        pool->destroy(ptr);
      }
    }
  };

  /**
   * Constructs an object of type T inside the memory pool and returns
   * it wrapped in a std::unique_ptr with a custom deleter.
   *
   * This function provides RAII-style lifetime management. The returned
   * unique_ptr automatically calls MemoryFactory::destroy() when it goes
   * out of scope, returning the memory slot back to the pool.
   *
   * Internally, this function delegates construction to create() and
   * then wraps the resulting raw pointer in a smart pointer.
   */
  template <class... Args>
  std::unique_ptr<T, PoolDeleter> make(Args &&...args) {
    T *obj = create(std::forward<Args>(args)...);
    return std::unique_ptr<T, PoolDeleter>(obj, PoolDeleter{this});
  }

  // Destroys the object and returns the slot to the free list
  void destroy(T *obj) noexcept {
    if (!obj)
      return;

    // Call the destructor of the object
    obj->~T();

    // Add the slot back to the free list
    Node *slot = reinterpret_cast<Node *>(obj);
    slot->next = freeList;
    freeList = slot;
  }

private:
  // A node in the free list, which can either hold a pointer to the next free
  // slot or the actual data
  union Node {
    Node *next;
    alignas(T) unsigned char data[sizeof(T)];
  };

  static constexpr std::size_t node_size = sizeof(Node);
  static constexpr std::size_t node_alignment = alignof(Node);

  static_assert(sizeof(Node) >= sizeof(T),
                "Node storage must be at least as large as T");
  static_assert(alignof(Node) >= alignof(T),
                "Node alignment must satisfy T's alignment requirement");

  // How many nodes in each allocated block
  std::size_t blockSize;

  // All allocated blocks of memory
  std::vector<void *> blocks;

  // Pointer to the head of the free list
  Node *freeList;

  /**
   * Constructs an object of type T inside pooled memory and returns
   * a raw pointer to it.
   *
   * This function removes a slot from the internal free list and uses
   * placement new to construct the object in-place. If no free slots are
   * available, a new block of memory is allocated.
   *
   * The caller is responsible for manually calling destroy() to return
   * the object to the pool. Failure to do so will result in the slot
   * not being reclaimed.
   **/
  template <class... Args> T *create(Args &&...args) {
    // If the free list is empty, allocate a new block of memory
    if (!freeList)
      allocateNewBlock();

    // Take the first node from the free list
    Node *slot = freeList;

    // Move the free list head to the next node
    freeList = freeList->next;

    // Construct a T object in the allocated slot using placement new
    return ::new (static_cast<void *>(slot)) T(std::forward<Args>(args)...);
  }

  void allocateNewBlock() {

    // Allocate a new block of memory for blockSize nodes, ensuring proper
    // alignment
    const std::size_t bytes = sizeof(Node) * blockSize;
    void *memory = ::operator new(bytes, std::align_val_t(alignof(Node)));
    blocks.push_back(memory);

    // Initialize the free list with the new block of nodes
    Node *block = static_cast<Node *>(memory);
    for (std::size_t i = 0; i < blockSize; ++i) {
      block[i].next = freeList;
      freeList = &block[i];
    }
  }
};

} // namespace cse498
