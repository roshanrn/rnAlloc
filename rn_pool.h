#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#define NUM_BINS 4

#if DEBUG_ALL
#define DEBUG(x) std::cout << "[DEBUG] " << x << std::endl;
#else
#define DEBUG(x)
#endif

class RnBackend;

class RnPageNode {
    public:
        RnPageNode() = default;
        void *addr_;
        size_t size_;

        bool operator==(const RnPageNode &other) const {
            return addr_ == other.addr_ && size_ == other.size_;
        }
};

class RnBin {
    public:
        RnBin() = default;
        RnBin(size_t size, std::shared_ptr<RnBackend> rnBackend)
            : rnBackend_(rnBackend),
              binSize_(size) {
            initialize_bin();
        };
        ~RnBin() {};

        void *binAllocate(size_t size);
        void binRelease(void *addr);

    private:
        void initialize_bin();
        void coalesceBinList();
        size_t binSize_;
        size_t num_;
        // std::vector<RnPageNode> binList_;
        std::list<RnPageNode> binList_;
        std::shared_ptr<RnBackend> rnBackend_;
};

class RnPool {
    public:
        RnPool() = default;
        RnPool(std::shared_ptr<RnBackend> rnBackend)
            : rnBackend_(rnBackend) {
            initialize_pool();
        };
        ~RnPool() {};

        void *poolAllocate(size_t size);
        void poolRelease(void *addr);

    private:
        void initialize_pool();

        size_t getBin(size_t size);
        std::shared_ptr<RnBackend> rnBackend_;
        std::unordered_map<size_t, std::unique_ptr<RnBin>> bins_;
        // size_t bin_size_class[NUM_BINS] = {128, 256};
        size_t bin_size_class[NUM_BINS] = {8, 16, 64, 128};
};
