#include "rn_allocator.h"
#include "rn_backend.h"
#include <cassert>
#include <pthread.h>

RnAllocator::RnAllocator(size_t size, size_t backend_page_size) {
    rnBackend_ = std::make_shared<RnBackend>(size, backend_page_size);
}

void *RnAllocator::rnAllocate(size_t size) {
    // Note that the actual design expects a cpu get
    // but we use thread id instead because of my system
    // limitations only 2 vCPUs.
    char name[16];
    pthread_getname_np(pthread_self(), name, 16);

    std::string tName(name);
    if (perCpuAllocator.find(tName) == perCpuAllocator.end()) {
        //std::cout << "[RnAllocator] Thread name is = " << tName << std::endl;
        std::unique_ptr<RnPool> rnPool = std::make_unique<RnPool>(rnBackend_);
        perCpuAllocator.insert(std::make_pair(tName, std::move(rnPool)));
    }
    return perCpuAllocator[tName]->poolAllocate(size);
}

void RnAllocator::rnDelete(void *addr) {
    assert(addr != nullptr);
    char name[16];
    pthread_getname_np(pthread_self(), name, 16);

    std::string tName(name);

    assert(perCpuAllocator.find(tName) != perCpuAllocator.end());
    perCpuAllocator[tName]->poolRelease(addr);
}
