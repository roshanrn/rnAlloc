#include "../rn_backend.h"
#include "../rn_pool.h"
#include <iostream>
#include <map>
#include <memory>

#define M 1 * 1024 * 1024

int main(int argc, char **argv) {

    std::vector<void *> testArr;
    testArr.resize(10);
    for (int i = 0; i < 10; i++) {
        testArr[i] = nullptr;
    }

    std::map<int, int> allocStats;
    std::map<int, int> freeStats;
    int skipped = 0;

    std::shared_ptr<RnBackend> rnBackend = std::make_shared<RnBackend>(4 * M);
    RnPool rnPool(rnBackend);

    for (int i = 0; i < 100; i++) {
        int op = rand() % 2;
        int idx = rand() % 10;

        if (op == 0 && testArr[idx] == nullptr) {
            testArr[idx] = rnPool.poolAllocate(128);
            allocStats[idx]++;
        } else if (op == 1 && testArr[idx] != nullptr) {
            rnPool.poolRelease(testArr[idx]);
            testArr[idx] = nullptr;
            freeStats[idx]++;
        } else {
            skipped++;
        }
    }

    std::cout << " *** Alloc Stats *** " << std::endl;
    for (auto it = allocStats.begin(); it != allocStats.end(); ++it) {
        if (it->second == 0)
            continue;
        std::cout << " Idx : " << it->first << " , Cnt: " << it->second
                  << std::endl;
    }

    std::cout << " *** Free Stats *** " << std::endl;
    for (auto it = freeStats.begin(); it != freeStats.end(); ++it) {
        if (it->second == 0)
            continue;
        std::cout << " Idx : " << it->first << " , Cnt: " << it->second
                  << std::endl;
    }

    std::cout << " *** Freeing Unreleased Mem *** " << std::endl;
    for (int i = 0; i < 10; i++) {
        if (testArr[i] != nullptr) {
            rnPool.poolRelease(testArr[i]);
            freeStats[i]++;
        }
    }

    std::cout << " *** Final Free Stats *** " << std::endl;
    for (auto it = freeStats.begin(); it != freeStats.end(); ++it) {
        if (it->second == 0)
            continue;
        std::cout << " Idx : " << it->first << " , Cnt: " << it->second
                  << std::endl;
    }
}
