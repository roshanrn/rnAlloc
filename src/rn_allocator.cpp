#include "rn_allocator.h"
#include "rn_backend.h"
#include <cassert>

RnAllocator::RnAllocator(size_t size, size_t backend_page_size) {
    rnBackend_ = std::make_shared<RnBackend>(size, backend_page_size);
}

void *RnAllocator::rnAllocate(size_t size) {
    // Note that the actual design expects a cpu get
    // but we use thread id instead because of my system
    // limitations only 2 vCPUs.
    std::thread::id this_id = std::this_thread::get_id();

    DEBUG("[RnAllocator] Alloc call of thread id " << this_id);

    if (perCpuAllocator.find(this_id) == perCpuAllocator.end()) {
        DEBUG("[RnAllocator] First alloc for this thread id " << this_id);
        std::unique_ptr<RnPool> rnPool = std::make_unique<RnPool>(rnBackend_);
        perCpuAllocator.insert(std::make_pair(this_id, std::move(rnPool)));
    }

    return perCpuAllocator[this_id]->poolAllocate(size);
}

void RnAllocator::rnDelete(void *addr) {
    assert(addr != nullptr);
    std::thread::id this_id = std::this_thread::get_id();
    perCpuAllocator[this_id]->poolRelease(addr);
}
