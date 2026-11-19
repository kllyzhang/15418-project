#pragma once

#include "csr.h"

#include <cstdint>
#include <vector>


/**
 * @brief sequentially solve Lx = b for vector x
 * @param[in] L     lower-triangular matrix
 * @param[in] b     dense vector
 * @param[in] x     output vector
 */
void solve_sequential(
    const CSRMatrix& L,
    const std::vector<double>& b,
    std::vector<double>& x
);
