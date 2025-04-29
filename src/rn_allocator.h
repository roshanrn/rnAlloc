#include "rn_pool.h"
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

#ifdef DEBUG_ALL
#define DEBUG(x) std::cout << "[DEBUG] " << x << std::endl;
#else
#define DEBUG(x)
#endif // DEBUi

class RnPool;
class RnBackend;

class RnAllocator {
    public:
        RnAllocator() = default;
        RnAllocator(size_t size, size_t backend_page_size = 2 * 1024 * 1024);

        void *rnAllocate(size_t size);
        void rnDelete(void *addr);

    private:
        // Even though the map is meant to be perCPU as the name suggests
        // this implementation uses it perThread (my VM has only 1 vCPU :()

        std::map<std::string, std::unique_ptr<RnPool>> perCpuAllocator;
        std::shared_ptr<RnBackend> rnBackend_;
};
