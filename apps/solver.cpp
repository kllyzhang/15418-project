#include "csr.h"
#include "sequential.h"
#include "level.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <unistd.h>

#include <omp.h>

#define ERROR_TOLERANCE 1e-8


using SolverSchedule = std::variant<
    LevelSchedule
>;


namespace SolveMethod {
    constexpr const char SEQUENTIAL = 'S';
    constexpr const char LEVELS = 'L';
    constexpr const char SYNC_SPARSE = 'Y';
    constexpr const char BARRIER_LIST = 'B';
}


int main(int argc, char *argv[]) {
    auto start_init = std::chrono::steady_clock::now();

    std::string input_path;
    int num_threads = 0;
    char solve_method = '\0';

    int opt;
    while ((opt = getopt(argc, argv, "f:n:m:")) != -1) {
        switch (opt) {
        case 'f':
            input_path = optarg;
            break;
        case 'n':
            num_threads = atoi(optarg);
            break;
        case 'm':
            solve_method = *optarg;
            break;
        default:
            std::cerr << "Usage: " << argv[0]
                      << " -f <path/to/input.csrbin> "
                      << "-n <num_threads> "
                      << "-m <solve_method>\n";
            exit(EXIT_FAILURE);
        }
    }

    if (input_path.empty() || num_threads <= 0 || 
        (solve_method != SolveMethod::SEQUENTIAL &&
         solve_method != SolveMethod::LEVELS &&
         solve_method != SolveMethod::SYNC_SPARSE &&
         solve_method != SolveMethod::BARRIER_LIST)) {
        std::cerr << "Usage: " << argv[0]
                  << " -f <path/to/input.csrbin> "
                  << "-n <num_threads> "
                  << "-m <solve_method>\n";
        exit(EXIT_FAILURE);
    }

    omp_set_num_threads(num_threads);

    CSRMatrix L;
    try {
        L = load_csr_matrix(input_path);
    } catch (const std::exception& e) {
        std::cerr << "Solver failed: "
                  << e.what() << '\n';
        exit(EXIT_FAILURE);
    }
    std::vector<double> solution(L.n, 1.0);
    std::vector<double> b = matvec(L, solution);
    std::vector<double> x;

    SolverSchedule schedule;
    switch (solve_method) {
    case SolveMethod::SEQUENTIAL:
        break;
    case SolveMethod::LEVELS:
        schedule = build_level_schedule(L);
        break;
    case SolveMethod::SYNC_SPARSE:
        std::cerr << "Solver failed: " 
                  << SolveMethod::SYNC_SPARSE << " not yet implemented\n";
        exit(EXIT_FAILURE);
    case SolveMethod::BARRIER_LIST:
        std::cerr << "Solver failed: " 
                  << SolveMethod::BARRIER_LIST << " not yet implemented\n";
        exit(EXIT_FAILURE);
    default:
        std::cerr << "Solver failed: " 
                  << "invalid solve_method " << solve_method << '\n';
        exit(EXIT_FAILURE);
    }

    double time_init = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - start_init).count();

    auto start_compute = std::chrono::steady_clock::now();

    switch (solve_method) {
    case SolveMethod::SEQUENTIAL:
        solve_sequential(L, b, x);
        break;
    case SolveMethod::LEVELS:
        solve_level(L, b, x, std::get<LevelSchedule>(schedule));
        break;
    case SolveMethod::SYNC_SPARSE:
        std::cerr << "Solver failed: " 
                  << SolveMethod::SYNC_SPARSE << " not yet implemented\n";
        exit(EXIT_FAILURE);
    case SolveMethod::BARRIER_LIST:
        std::cerr << "Solver failed: " 
                  << SolveMethod::BARRIER_LIST << " not yet implemented\n";
        exit(EXIT_FAILURE);
    default:
        std::cerr << "Solver failed: " 
                  << "invalid solve_method " << solve_method << '\n';
        exit(EXIT_FAILURE);
    }

    double time_compute = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - start_compute).count();
    
    double max_error = 0.0;
    for (uint32_t i = 0; i < L.n; i++) {
        double error = std::abs(x[i] - solution[i]);
        if (!std::isfinite(error)) {
            max_error = error;
            break;
        }
        max_error = std::max(max_error, error);
    }

    bool is_correct = std::isfinite(max_error) && max_error < ERROR_TOLERANCE;

    std::cout << std::setprecision(10)
              << "Matrix:               " << input_path << '\n'
              << "Rows:                 " << L.n << '\n'
              << "Nonzeros:             " << L.num_non_zeroes() << '\n'
              << "Method:               " << solve_method << '\n'
              << "Threads:              " << num_threads << '\n'
              << "Init. Time (ms):      " << time_init << '\n'
              << "Compute Time (ms):    " << time_compute << '\n'
              << "Max error:            " << max_error << '\n'
              << "Correct:              " << (is_correct ? "PASS" : "FAIL") << '\n';

    return 0;
}
