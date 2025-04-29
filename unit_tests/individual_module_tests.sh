#!/bin/bash

if [ ! -d "tests" ]; then
	mkdir "tests"
fi

g++ -w test_backend.cpp ../src/rn_backend.cpp -o tests/test_backend
g++ -w test_pool_and_backend.cpp ../src/rn_backend.cpp ../src/rn_pool.cpp -o tests/test_backend_and_pool
g++ -w test_all.cpp ../src/rn_allocator.cpp ../src/rn_backend.cpp ../src/rn_pool.cpp -o tests/test_all
g++ -w test_all_multi.cpp ../src/rn_allocator.cpp ../src/rn_backend.cpp ../src/rn_pool.cpp -o tests/test_multi
g++ -w test_cross_cpu_free.cpp ../src/rn_backend.cpp ../src/rn_pool.cpp -o tests/test_cross_cpu_free

