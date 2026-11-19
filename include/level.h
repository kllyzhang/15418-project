#pragma once

#include "csr.h"

#include <cstdint>
#include <vector>


/** 
 * @brief level-synchronous parallelism schedule representation
 */
struct LevelSchedule {
    std::vector<uint32_t> level_ptr;
    std::vector<uint32_t> rows;
    std::size_t num_levels() const {
        return level_ptr.size() - 1;}
};


/**
 * @brief construct level-synchronous parallelism schedule
 * @param[in] L         lower-triangular matrix
 * @return LevelSchedule object
 */
LevelSchedule build_level_schedule(const CSRMatrix& L);


/**
 * @brief level-synchronous parallel solve Lx = b for vector x
 * @param[in] L         lower-triangular matrix
 * @param[in] b         dense vector
 * @param[in] x         output vector
 * @param[in] schedule  parallelism schedule
 */
void solve_level(
    const CSRMatrix& L,
    const std::vector<double>& b,
    std::vector<double>& x,
    const LevelSchedule& schedule
);
