#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <sys/mman.h>
#include <vector>

#ifdef DEBUG_ALL
#define DEBUG(x) std::cout << "[DEBUG] " << x << std::endl;
#else
#define DEBUG(x)
#endif // DEBUi

class BackendPage {
    public:
        void *addr_;
        size_t size_;
};
typedef BackendPage bPage;

class RnBackend {
    public:
        RnBackend() = default;
        RnBackend(size_t backendSize, size_t pageSize = 2 * 1024 * 1024)
            : backendSize_(backendSize),
              pageSize_(pageSize),
              numPages_(backendSize / pageSize),
              numFreePages_(numPages_),
              numUsedPages_(0) {
            metaPageMap.clear();
            if (!initBackend()) {
                throw std::runtime_error("Failed to initialize memory backend");
            }
        };
        ~RnBackend();

        bool initBackend();
        void *allocatePage(size_t binSize, size_t &allocSize);
        void deallocatePage(void *addr) {
            // TODO: Currently don't support releasing memory from a pool to the
            // backend
        };

        size_t getFreeSize(void *addr);

    private:
        std::mutex mutex_;
        void *mem_ = nullptr;
        size_t backendSize_;
        size_t pageSize_;
        size_t numPages_;
        std::deque<bPage> freePages_;
        std::deque<bPage> usedPages_;
        size_t numFreePages_;
        size_t numUsedPages_;
        std::map<void *, size_t> metaPageMap; // make radix tree for scalibility
};
