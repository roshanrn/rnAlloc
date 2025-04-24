// #include "rn_allocator.h"
#include "../rn_allocator.h"
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <thread>
#include <vector>

#define NUM_SLOTS 5
#define NUM_ITERS 10
#define ALLOC_SIZE 128
#define NUM_THREADS 2

// std::map<int, int> allocStats;
// std::map<int, int> freeStats;

int allocStats[NUM_THREADS][NUM_SLOTS];
int freeStats[NUM_THREADS][NUM_SLOTS];

void per_thread_bench(RnAllocator *rnAllocator, int tId) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, NUM_SLOTS - 1);

    std::vector<void *> allocated_memory;
    allocated_memory.resize(NUM_SLOTS);

    for (int i = 0; i < NUM_SLOTS; i++) {
        allocated_memory[i] = nullptr;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_ITERS; ++i) {
        int op = distrib(gen);
        int index = distrib(gen) % NUM_SLOTS;

        // Randomly choose to allocate or deallocate
        if (op % 2 == 0 && allocated_memory[index] == nullptr) {
            std::cout << " Alloc Call - " << std::endl;
            allocated_memory[index] = rnAllocator->rnAllocate(ALLOC_SIZE);
            allocStats[tId][index]++;
        } else if (allocated_memory[index] != nullptr) {
            std::cout << " Free Call - " << std::endl;
            rnAllocator->rnDelete(allocated_memory[index]);
            freeStats[tId][index]++;
            allocated_memory[index] = nullptr;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time - start_time)
                        .count();

    // Clean up any remaining allocated memory
    for (int i = 0; i < NUM_SLOTS; i++) {
        if (allocated_memory[i] != nullptr) {
            rnAllocator->rnDelete(allocated_memory[i]);
            freeStats[tId][i]++;
        }
    }

    std::cout << "Thread Id " << tId << " duration = " << duration << std::endl;
}

void run_bench(int num_threads, RnAllocator *rnAllocator) {

    std::vector<std::thread> threads;

    auto total_start_time = std::chrono::high_resolution_clock::now();

    // Create and start threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(per_thread_bench, rnAllocator, i);
    }

    // Wait for threads to finish
    for (auto &thread : threads) {
        thread.join();
    }

    auto total_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                              total_end_time - total_start_time)
                              .count();

    std::cout << "Total Time with " << num_threads
              << " threads: " << total_duration << " ms" << std::endl;

    std::cout << " ****** Alloc Stats ****** " << std::endl;
    for (int i = 0; i < NUM_THREADS; i++) {
        std::cout << " ## THREAD " << i << " Alloc ##" << std::endl;
        for (int j = 0; j < NUM_SLOTS; j++) {
            if (allocStats[i][j] == 0) {
                continue;
            }
            std::cout << " Idx : " << j << " , Cnt: " << allocStats[i][j]
                      << std::endl;
        }
    }

    std::cout << " ****** Free Stats ****** " << std::endl;
    for (int i = 0; i < NUM_THREADS; i++) {
        std::cout << " ## THREAD " << i << " Free ##" << std::endl;
        for (int j = 0; j < NUM_SLOTS; j++) {
            if (freeStats[i][j] == 0) {
                continue;
            }
            std::cout << " Idx : " << j << " , Cnt: " << freeStats[i][j]
                      << std::endl;
        }
    }
}

int main(int argc, char **argv) {

    for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < NUM_SLOTS; j++) {
            allocStats[i][j] = 0;
            freeStats[i][j] = 0;
        }
    }

    RnAllocator rnAllocator(8 * 1024 * 1024);
    run_bench(NUM_THREADS, &rnAllocator);
}
