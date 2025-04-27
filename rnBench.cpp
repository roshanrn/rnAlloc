#include "src/rn_allocator.h"
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <iostream>
#include <random>
#include <sys/sysinfo.h>
#include <thread>
#include <vector>

#define NUM_SIZES 4
#define NUM_OPS 2
#define NUM_THREADS 4

int ALLOC_SIZES[NUM_SIZES]{16, 64, 128, 256};

void per_thread_bench(RnAllocator *rnAllocator, int tId, int num_slots,
                      int num_iters) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> idxDistrib(0, num_slots - 1);
    std::uniform_int_distribution<> opDistrib(0, NUM_OPS - 1);
    std::uniform_int_distribution<> sizeDistrib(0, NUM_SIZES - 1);

    std::vector<std::vector<void *>> allocated_memory(NUM_SIZES);
    for (int i = 0; i < NUM_SIZES; i++) {
        allocated_memory[i].resize(num_slots);
    }

    for (int i = 0; i < NUM_SIZES; i++) {
        for (int j = 0; j < num_slots; j++) {
            allocated_memory[i][j] = nullptr;
        }
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_iters; ++i) {
        int op = opDistrib(gen);
        int index = idxDistrib(gen) % num_slots;
        int sizeIdx;

        if (NUM_SIZES == 1) {
            sizeIdx = 0;
        } else {
            sizeIdx = sizeDistrib(gen) % NUM_SIZES;
        }

        // Randomly choose to allocate or deallocate
        if (op % 2 == 0 && allocated_memory[sizeIdx][index] == nullptr) {
            if (rnAllocator == nullptr) {
                allocated_memory[sizeIdx][index] = malloc(ALLOC_SIZES[sizeIdx]);
            } else {
                allocated_memory[sizeIdx][index] =
                    rnAllocator->rnAllocate(ALLOC_SIZES[sizeIdx]);
            }
        } else if (allocated_memory[sizeIdx][index] != nullptr) {
            if (rnAllocator == nullptr) {
                free(allocated_memory[sizeIdx][index]);
            } else {
                rnAllocator->rnDelete(allocated_memory[sizeIdx][index]);
            }
            allocated_memory[sizeIdx][index] = nullptr;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time - start_time)
                        .count();

    // Clean up any remaining allocated memory
    for (int i = 0; i < NUM_SIZES; i++) {
        for (int j = 0; j < num_slots; j++) {
            if (allocated_memory[i][j] != nullptr) {
                if (rnAllocator == nullptr) {
                    free(allocated_memory[i][j]);
                } else {
                    rnAllocator->rnDelete(allocated_memory[i][j]);
                }
            }
        }
    }

    std::cout << "Thread Id " << tId << " duration = " << duration << std::endl;
}

void run_bench(int num_threads, int num_slots, int num_iters,
               RnAllocator *rnAllocator) {

    std::vector<std::thread> threads;

    auto total_start_time = std::chrono::high_resolution_clock::now();

    // Create and start threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(per_thread_bench, rnAllocator, i, num_slots,
                             num_iters);
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
}

int main(int argc, char **argv) {
    bool runRnAlloc(false), runMalloc(false);
    int num_threads = NUM_THREADS, num_slots = 100000, num_iters = 500000;
    std::cout << " Number of arguments = " << argc << std::endl;
    if (argc == 6) {
        num_threads = std::atoi(argv[1]);
        num_slots = std::atoi(argv[2]);
        num_iters = std::atoi(argv[3]);
        if (std::string(argv[4]) == "yes") {
            runRnAlloc = true;
        }
        if (std::string(argv[5]) == "yes") {
            runMalloc = true;
        }
        std::cout << "Using NumThreads = " << num_threads
                  << " and NumSlots = " << num_slots
                  << " and NumIteratios = " << num_iters
                  << " and rnAlloc and linked allocator = " << runRnAlloc
                  << std::endl;
    } else {
        runRnAlloc = true;
        runMalloc = true;
        std::cout << "Using defaults of: NumThreads = " << num_threads
                  << " and NumSlots = " << num_slots
                  << " and NumIteratios = " << num_iters
                  << " and running both rnAlloc and linked allocator"
                  << std::endl;
    }

    if (runRnAlloc) {
        size_t total_mem_estimate, per_bin_estimate = 0, sum_sizes = 0;
        for (int i = 0; i < NUM_SIZES; i++) {
            sum_sizes += ALLOC_SIZES[i];
        }
        struct sysinfo ramInfo;
        if (sysinfo(&ramInfo) != 0) {
            std::cerr << "Unable to get free RAM info " << std::strerror(errno)
                      << std::endl;
            return 1;
        }
        size_t free_memory = ramInfo.freeram;
        total_mem_estimate = num_threads * num_slots * sum_sizes;
        //std::cout << "Free Memory = " << free_memory
        //          << " Mem Estimate = " << total_mem_estimate << std::endl;
        total_mem_estimate =
            (total_mem_estimate +4096 * 4096 - 1) / (4096 * 4096);
        total_mem_estimate = total_mem_estimate * 4096 * 4096;

        //std::cout << " Mem Estimate = " << total_mem_estimate << std::endl;

        if (free_memory / 2 < total_mem_estimate) {
            std::cout
                << " Not enough free memory, reduce slots or threads or both"
                << " Free mem = " << free_memory << std::endl;
            return 1;
        }

        RnAllocator rnAllocator(total_mem_estimate,
                                total_mem_estimate / (NUM_SIZES * num_threads));
        // RnAllocator rnAllocator(768 * 1024 * 1024, 10 * 1024 * 1024);
        run_bench(num_threads, num_slots, num_iters, &rnAllocator);
    }

    if (runMalloc) {
        run_bench(num_threads, num_slots, num_iters, nullptr);
    }
}
