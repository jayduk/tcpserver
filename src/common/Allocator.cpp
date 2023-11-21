
#include <memory>
void f()
{
    std::allocator<char> a;

    void* p = a.allocate(1);
}