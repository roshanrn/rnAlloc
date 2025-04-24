#pragma once
#include "rn_pool.h"
#include "rn_backend.h"
#include <algorithm>
#include <cassert>
#include <memory>

void RnBin::initialize_bin() {
    DEBUG("[RnBin] Initializing Bin of Size = " << binSize_);

    size_t allocSize;
    void *ptr = rnBackend_->allocatePage(binSize_, allocSize);
    binList_.push_back({ptr, allocSize});

    DEBUG("[RnBin] RnBacked gave Bin a page with addr = "
          << ptr << " and alloc size " << allocSize);
}

void RnBin::coalesceBinList() {
#if 0
    std::sort(binList_.begin(), binList_.end(),
              [](const RnPageNode &a, const RnPageNode &b) {
                  return a.addr_ < b.addr_;
              });
#endif
    DEBUG("[RnBin] After sorting Bin List has " << binList_.size()
                                                << " elements");
    /*
    for (int i = 0; i < binList_.size(); i++) {
        std::cout << " Addr in list at " << i << " is " << binList_[i].addr_
                  << " and the size i " << binList_[i].size_ << std::endl;
    }
    */
    // Merge intervals
}

void *RnBin::binAllocate(size_t size) {
    if (binList_.size() == 0) {
        initialize_bin();
    }

    RnPageNode currPage = *binList_.begin();
    size_t currSize = currPage.size_;
    void *currAddr = currPage.addr_;

#if 0
    DEBUG("[RnBin] Size is " << binList_.size());
    binList_.erase(binList_.begin());
    DEBUG("[RnBin] Size is " << binList_.size());

    char *newAddr = static_cast<char *>(currAddr) + binSize_;
    size_t newSize = currSize - binSize_;

    if (newSize != 0) {
        binList_.push_back({(void *)newAddr, newSize});
    }
#else
    DEBUG("[RnBin] Size is " << binList_.size());

    char *newAddr = static_cast<char *>(currAddr) + binSize_;
    size_t newSize = currSize - binSize_;

    if (newSize == 0) {
        binList_.erase(binList_.begin());
        DEBUG("[RnBin] Size is " << binList_.size());
    } else {
        auto it = binList_.begin();
        (*it).size_ = newSize;
        (*it).addr_ = (void *)newAddr;
    }
#endif

    DEBUG("[RnBin] Bin is allocating " << binSize_ << " bytes at addr "
                                       << currAddr << " and new addr is "
                                       << (void *)newAddr);

    return currAddr;
}

void RnBin::binRelease(void *addr) {
    binList_.push_back({addr, binSize_});
    // coalesceBinList();
}

void RnPool::initialize_pool() {
    DEBUG("Initializing Pool");

    for (size_t i = 0; i < NUM_BINS; i++) {
        size_t binSize = bin_size_class[i];
        std::unique_ptr rnBin = std::make_unique<RnBin>(binSize, rnBackend_);
        bins_[i] = std::move(rnBin);
    }
}

size_t RnPool::getBin(size_t size) {
    size_t ret = 100000;
    for (int i = 0; i < NUM_BINS; i++) {
        if (bin_size_class[i] == size) {
            ret = i;
            break;
        }
    }

    return ret;
}

void *RnPool::poolAllocate(size_t size) {
    size_t binId = getBin(size);
    DEBUG("[RnPool] The bin id selected for alloc is " << binId);

    return bins_[binId]->binAllocate(size);
}

void RnPool::poolRelease(void *addr) {
    assert(addr != nullptr);

    size_t binSize = rnBackend_->getFreeSize(addr);
    size_t binId = getBin(binSize);
    DEBUG("[RnPool] The bin id selected for release is " << binId);

    bins_[binId]->binRelease(addr);
}
