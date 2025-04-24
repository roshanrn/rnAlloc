#!/bin/bash

if [ ! -d "tests" ]; then
	mkdir "tests"
fi

g++ -w test_backend.cpp ../rn_backend.cpp -o tests/test_backend
g++ -w test_pool_and_backend.cpp ../rn_backend.cpp ../rn_pool.cpp -o tests/test_backend_and_pool
g++ -w test_all.cpp ../rn_allocator.cpp ../rn_backend.cpp ../rn_pool.cpp -o tests/test_all
g++ -w test_all_multi.cpp ../rn_allocator.cpp ../rn_backend.cpp ../rn_pool.cpp -o tests/test_multi
