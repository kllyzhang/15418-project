#include "level.h"

#include <algorithm>
#include <iostream>


LevelSchedule build_level_schedule(const CSRMatrix& L) {
    std::vector<uint32_t> levels(L.n, 0);
    uint32_t max_level = 0;

    for (uint32_t i = 0; i < L.n; i++) {
        for (uint32_t j = L.row_ptr[i]; j < L.row_ptr[i + 1] - 1; j++) {
            levels[i] = std::max(levels[i], levels[L.col_idx[j]] + 1);
        }
        max_level = std::max(max_level, levels[i]);
    }

    LevelSchedule schedule;
    schedule.level_ptr.assign(max_level + 2, 0);
    schedule.rows.resize(L.n);

    for (uint32_t i = 0; i < L.n; i++) {
        schedule.level_ptr[levels[i] + 1]++;
    }
    for (uint32_t i = 1; i < schedule.level_ptr.size(); i++) {
        schedule.level_ptr[i] += schedule.level_ptr[i - 1];
    }

    std::vector<uint32_t> offsets = schedule.level_ptr;

    for (uint32_t i = 0; i < L.n; i++) {
        uint32_t level = levels[i];
        schedule.rows[offsets[level]++] = i;
    }
    
    return schedule;
}


void solve_level(
        const CSRMatrix& L,
        const std::vector<double>& b,
        std::vector<double>& x,
        const LevelSchedule& schedule) {
    x.resize(L.n);

    #pragma omp parallel
    {
    
    for (size_t level = 0; level < schedule.num_levels(); level++) {
        const uint32_t level_start = schedule.level_ptr[level];
        const uint32_t level_end = schedule.level_ptr[level + 1];

        #pragma omp for schedule(static)
        for (uint32_t k = level_start; k < level_end; k++) {
            uint32_t i = schedule.rows[k];
            double t = b[i];

            const uint32_t start = L.row_ptr[i];
            const uint32_t end = L.row_ptr[i + 1];
            
            for (uint32_t j = start; j < end - 1; j++) {
                t -= L.values[j] * x[L.col_idx[j]];
            }

            x[i] = t / L.values[end - 1];
        }
    }

    }
}
