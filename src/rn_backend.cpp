#include "rn_backend.h"
#include <cerrno>
#include <cstring>
#include <memory>
#include <utility>

bool RnBackend::initBackend() {
    mem_ = mmap(nullptr, backendSize_, PROT_READ | PROT_WRITE,
                MAP_ANON | MAP_PRIVATE, -1, 0);

    if (!mem_) {
        std::cout << "ERROR: mmap() failed!" << std::endl;
        return false;
    }

    DEBUG("[RnBackend] Successfully Allocated "
          << backendSize_ << " via mmap. Addr is " << mem_);

    freePages_.push_back({mem_, backendSize_});
    return true;
}

RnBackend::~RnBackend() {
    if (munmap(mem_, backendSize_) == -1) {
        std::cerr << "Munmap failed " << std::strerror(errno) << std::endl;
    }
    DEBUG("[RnBackend] Successfully released mmap-ed memory");
}

void *RnBackend::allocatePage(size_t binSize, size_t &allocSize) {
    std::unique_lock<std::mutex> lock(mutex_);

    if (freePages_.size() == 0) {
        std::cout << "Found 0 Free Pages in RnBackend" << std::endl;
        return nullptr;
    }

    bPage &allocatedPage = freePages_.front();
    void *addr = allocatedPage.addr_;
    size_t s = allocatedPage.size_;

    bPage currPage = freePages_.front();
    freePages_.pop_front();

    char *newAddr = static_cast<char *>(currPage.addr_);
    newAddr += pageSize_;

    metaPageMap.insert(std::make_pair(currPage.addr_, binSize));
    allocSize = pageSize_;

    size_t newSize = s - pageSize_;
    if (newSize != 0) {
        freePages_.push_back({newAddr, newSize});
    }

    return currPage.addr_;
}

size_t RnBackend::getFreeSize(void *ptr) {
    size_t ret = 0;
    char *toFind = static_cast<char *>(ptr);

    for (auto it = metaPageMap.begin(); it != metaPageMap.end(); it++) {
        char *cmpr = static_cast<char *>(it->first);
        if (toFind == cmpr || toFind < cmpr + pageSize_) {
            ret = it->second;
            break;
        }
    }

    DEBUG("[RnPool] Free Size API, size is " << ret);

    return ret;
}
