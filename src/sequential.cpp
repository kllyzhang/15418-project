#include "sequential.h"

#include <cstdint>


void solve_sequential(
        const CSRMatrix& L, 
        const std::vector<double>& b, 
        std::vector<double>& x) {
    x.resize(L.n);

    for (uint32_t i = 0; i < L.n; i++) {
        double t = b[i];

        const uint32_t start = L.row_ptr[i];
        const uint32_t end = L.row_ptr[i + 1];

        for (uint32_t j = start; j < end - 1; j++) {
            t -= L.values[j] * x[L.col_idx[j]];
        }

        x[i] = t / L.values[end - 1];
    }
}
