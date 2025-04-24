#include "../rn_backend.h"
#include <iostream>

#define M 1 * 1024 * 1024

int main(int argc, char **argv) {
    RnBackend rnBackend(4 * M);

    size_t allocS;
    void *a = rnBackend.allocatePage(16, allocS);
    std::cout << "Addr allocated is ptr = " << a << " and size is = " << allocS
              << std::endl;

    a = rnBackend.allocatePage(16, allocS);
    std::cout << "Addr allocated is ptr = " << a << " and size is = " << allocS
              << std::endl;
}
