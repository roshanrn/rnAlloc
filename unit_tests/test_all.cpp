#include "../src/rn_allocator.h"
#include <iostream>
#include <map>
#include <memory>

#define M 1 * 1024 * 1024
#define NUM_ITERS 2
#define NUM_SLOTS 2

int main(int argc, char **argv) {

    std::vector<void *> testArr;
    testArr.resize(NUM_SLOTS);
    for (int i = 0; i < NUM_SLOTS; i++) {
        testArr[i] = nullptr;
    }

    std::map<int, int> allocStats;
    std::map<int, int> freeStats;
    int skipped = 0;

    RnAllocator rnAllocator(2 * M);

    for (int i = 0; i < NUM_ITERS; i++) {
        int op = rand() % 2;
        int idx = rand() % NUM_SLOTS;

        if (op == 0 && testArr[idx] == nullptr) {
            testArr[idx] = rnAllocator.rnAllocate(128);
            allocStats[idx]++;
        } else if (op == 1 && testArr[idx] != nullptr) {
            rnAllocator.rnDelete(testArr[idx]);
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
    for (int i = 0; i < NUM_SLOTS; i++) {
        if (testArr[i] != nullptr) {
            rnAllocator.rnDelete(testArr[i]);
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
