# Parallel Sparse Triangular Solve

[Proposal](reports/proposal.pdf) &middot; Milestone Report &middot; Final Report

## Summary

This project investigates parallel scheduling strategies for sparse triangular solves (SpTRSV) on multicore CPUs using an OpenMP programming model. 


## Quick Start

### Build

Requires g++ with C++17 and OpenMP support.
```
make csr-join   # reconstruct CSR matrices from git files (saves to data/input/cache/)
make            # build executables
make clean      # remove compiled objects and executables
```

### Run

#### Convert matrix market (`.mtx`) files to binary CSR (`.csrbin`) format:
```
./bin/converter <path/to/input.mtx> <path/to/output.csrbin>
```

#### Solve a sparse triangular system:
```
./bin/solver -f <path/to/input.csrbin> -n <num_threads> -m <solve_method>
```

Example:
```
./bin/solver -f data/input/cache/apache2.csrbin -n 1 -m S
```

Flags:
| Flag | Description |
|---|---|
| `-f` | Path to input (.csrbin) file |
| `-n` | Number of OpenMP threads (>0)
| `-m` | Solving method: `S` (Sequential); `L` (Levels Synchronization); `Y` (Synchronization sparsification); `B` (Barrier List)


## Code Structure

```
15418-project/
├── apps/           # executables source code
├── benchmark/      # benchmark scripts and analysis
├── data/
│   ├── input/      # CSR matrices, chunks, and original datasets
│   └── output/     # generated results
├── include/        # C++ headers
├── reports/        # project reports
├── src/            # solver and matrix implementations
└── Makefile        # prepare data and build executables
```

