#include "rn_allocator.h"
#include "rn_backend.h"
#include <cassert>
#include <sstream>

RnAllocator::RnAllocator(size_t size, size_t backend_page_size,
                         bool setAffinity) {
    rnBackend_ = std::make_shared<RnBackend>(size, backend_page_size);
    setAffinity_ = setAffinity;
    tCnt = 1;
}

void *RnAllocator::rnAllocate(size_t size) {
    // Note that the actual design expects a cpu get
    // but we use thread id instead because of my system
    // limitations only 2 vCPUs.

    // DEBUG("[RnAllocator] Alloc call of thread id " << this_id);
    std::thread::id this_id = std::this_thread::get_id();
    void *ptr = nullptr;

#if 1
    uint64_t id = 0;
    if (!setAffinity_) {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        id = std::stoull(ss.str());
    } else {
        id = sched_getcpu();
    }

    if (perCpuAllocator.find(id) == perCpuAllocator.end()) {
        if (setAffinity_)
            std::cout << " CPU affinity = " << id << std::endl;
        std::unique_ptr<RnPool> rnPool =
            std::make_unique<RnPool>(rnBackend_, id);
        perCpuAllocator.insert(std::make_pair(id, std::move(rnPool)));
    }
    ptr = perCpuAllocator[id]->poolAllocate(size);
    return ptr;
#else
    if (threadMap_.find(this_id) == threadMap_.end()) {
        DEBUG("[RnAllocator] Set thread id for " << this_id << " as " << tCnt);
        threadMap_[this_id] = tCnt;
        std::unique_ptr<RnPool> rnPool =
            std::make_unique<RnPool>(rnBackend_, tCnt);
        perCpuAllocator.insert(std::make_pair(tCnt, std::move(rnPool)));
        tCnt++;
    }

    /*
    if (perCpuAllocator.find(this_id) == perCpuAllocator.end()) {
        DEBUG("[RnAllocator] First alloc for this thread id " << this_id);
        std::unique_ptr<RnPool> rnPool = std::make_unique<RnPool>(rnBackend_);
        perCpuAllocator.insert(std::make_pair(this_id, std::move(rnPool)));
    }

    return perCpuAllocator[this_id]->poolAllocate(size);
    */

    int tId = threadMap_[this_id];
    ptr = perCpuAllocator[tId]->poolAllocate(size);
    return ptr;
#endif
}

void RnAllocator::rnDelete(void *addr) {
    assert(addr != nullptr);
    std::thread::id this_id = std::this_thread::get_id();

#if 1
    uint64_t id = 0;
    if (!setAffinity_) {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        id = std::stoull(ss.str());
    } else {
        id = sched_getcpu();
    }

    // assert(perCpuAllocator.find(id) != perCpuAllocator.end());
    perCpuAllocator[id]->poolRelease(addr);
#else
    assert(threadMap_.find(this_id) != threadMap_.end());
    int tId = threadMap_[this_id];
    perCpuAllocator[tId]->poolRelease(addr);
#endif
}
