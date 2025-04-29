#include "../src/rn_backend.h"
#include "../src/rn_pool.h"
#include <iostream>
#include <memory>

int main(int argc, char **argv) {

    std::shared_ptr<RnBackend> rnBackend =
        std::make_shared<RnBackend>(32 * 1024, 256);

    /* Using Size = 8 KB and PageSize = 256 bytes configuration in
       RnBackend constructor, when a RnPool is initialized, each RnBin gets 256
       bytes in the RnList. Then we do the below sequence:
        1. Allocate 256 bytes from pool1. The 256 byte RnBin in Pool1 will now
       have 0 space left.
        2. Use Pool2 to free this addr. This addr gets added to the 256 byte
       size RnBin of Pool2. In the current RnBin code, whatever is released is
       added at the end of the list. This means now the 256 byte RnBin in Pool2
       has two elements each of 256 bytes. The addr allocated by Pool1 is at the
       end.
        3. Do a 256 bytes alloc to discard from the 256byte RnBin of Pool 2.
        4. Allocate 256 bytes again using Pool2. We get the same addr as the
       allocation in step 1. from pool 1.
        5. This verifies that cross cpu free's are possible with the current
       design and code.
    */

    RnPool pool1 = RnPool(rnBackend);
    RnPool pool2 = RnPool(rnBackend);

    void *ptrPool1 = pool1.poolAllocate(256);
    std::cout << " Add allocated by pool 1 is " << ptrPool1 << std::endl;

    std::cout << " Freeing mem allocated by pool 1 using pool 2" << std::endl;
    pool2.poolRelease(ptrPool1);

    void *ptrPool2 = pool2.poolAllocate(256);
    std::cout << " Discard Addr allocated by pool 2 is " << ptrPool2
              << std::endl;
    ptrPool2 = pool2.poolAllocate(256);

    std::cout << " Addr allocated by pool 2 is " << ptrPool2 << std::endl;
    return 0;
}
