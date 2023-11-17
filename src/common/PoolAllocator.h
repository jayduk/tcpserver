#ifndef COMMON_POOLALLOCATOR_H_
#define COMMON_POOLALLOCATOR_H_

#include <memory>
#include <vector>
template<typename basic_alloc = std::allocator<void>>
class PoolAllocator
{
    typedef int free_list;

private:
    static thread_local PoolAllocator* instance_;

    static thread_local free_list* free_list_;

public:
    PoolAllocator();
    ~PoolAllocator() = default;
};

#endif  // COMMON_POOLALLOCATOR_H_