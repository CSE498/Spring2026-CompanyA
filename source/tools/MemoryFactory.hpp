/**
 * @brief A memory manager class that pre-allocates blocks of memory for
 *  a specific type T and manages a free list for efficient allocation and deallocation.
 * @note Status: PROPOSAL
 **/

 // Refactored this class to use smart pointers using an LLM

#pragma once
#include <memory>
#include <vector>
#include <cstddef>
#include <utility>
#include <cassert>
#include <new>

namespace cse498 {

    // Templated to work for any type T
    template <class T>
    class MemoryFactory {

    public:

        explicit MemoryFactory(std::size_t blockSize = 64)
            : blockSize(blockSize), freeList(nullptr)
        {
            assert(blockSize > 0);
            allocateNewBlock();
        }

        MemoryFactory(const MemoryFactory&) = delete;
        MemoryFactory& operator=(const MemoryFactory&) = delete;

        // Destructor to free all allocated blocks of memory
        ~MemoryFactory() {
            for (void* block : blocks) {
                ::operator delete(block, std::align_val_t(alignof(Node)));
            }
        }

        struct PoolDeleter {
            MemoryFactory* pool{nullptr};

            void operator()(T* ptr) const noexcept {
                if (pool && ptr) {
                    pool->destroy(ptr);
                }
            }
        };

        // Creates an object of type T with the given arguments and returns
        // a unique_ptr that will return the memory to the pool when destroyed
        template <class... Args>
        std::unique_ptr<T, PoolDeleter> make(Args&&... args) {
            T* obj = create(std::forward<Args>(args)...);
            return std::unique_ptr<T, PoolDeleter>(obj, PoolDeleter{this});
        }

        
        template <class... Args>
        T* create(Args&&... args) {
            // If the free list is empty, allocate a new block of memory
            if (!freeList) allocateNewBlock();

            // Take the first node from the free list
            Node* slot = freeList;

            // Move the free list head to the next node
            freeList = freeList->next;

            // Construct a T object in the allocated slot using placement new
            return ::new (static_cast<void*>(slot)) T(std::forward<Args>(args)...);
        }

        // Destroys the object and returns the slot to the free list
        void destroy(T* obj) noexcept {
            if (!obj) return;
            
            // Call the destructor of the object
            obj->~T();

            // Add the slot back to the free list
            Node* slot = reinterpret_cast<Node*>(obj);
            slot->next = freeList;
            freeList = slot;
        }

    private:

        // A node in the free list, which can either hold a pointer to the next free slot or the actual data
        union Node {
            Node* next;
            alignas(T) unsigned char data[sizeof(T)];
        };

        static_assert(sizeof(Node)  >= sizeof(T), "Node storage must be at least as large as T");
        static_assert(alignof(Node) >= alignof(T), "Node alignment must satisfy T's alignment requirement");
        
        // How many nodes in each allocated block
        std::size_t blockSize;

        // All allocated blocks of memory
        std::vector<void*> blocks;

        // Pointer to the head of the free list
        Node* freeList;

        void allocateNewBlock() {

            // Allocate a new block of memory for blockSize nodes, ensuring proper alignment
            const std::size_t bytes = sizeof(Node) * blockSize;
            void* memory = ::operator new(bytes, std::align_val_t(alignof(Node)));
            blocks.push_back(memory);

            // Initialize the free list with the new block of nodes
            Node* block = static_cast<Node*>(memory);
            for (std::size_t i = 0; i < blockSize; ++i) {
                block[i].next = freeList;
                freeList = &block[i];
            }
        }
    };

} 
