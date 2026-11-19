#include "csr.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>


namespace {
    const char CSR_MAGIC[8] = {
        'C', 'S', 'R', 'S', 'P', 'T', 'R', '1'
    };


    struct Entry {
        uint32_t row;
        uint32_t col;
        double value;
    };
}


namespace MatrixMarket {
    constexpr const char* HEADER = "%%MatrixMarket";

    constexpr const char* MATRIX = "matrix";
    constexpr const char* COORDINATE = "coordinate";
    
    constexpr const char* REAL = "real";
    constexpr const char* INTEGER = "integer";
    constexpr const char* PATTERN = "pattern";

    constexpr const char* GENERAL = "general";
    constexpr const char* SYMMETRIC = "symmetric";
}


CSRMatrix build_csr_matrix(const uint32_t n, const std::vector<Entry>& entries) {
    CSRMatrix A;
    A.n = n;
    A.row_ptr.assign(n + 1, 0);

    for (const auto& entry : entries) {
        A.row_ptr[entry.row + 1]++;
    }

    for (size_t i = 1; i < n + 1; i++) {
        A.row_ptr[i] += A.row_ptr[i - 1];
    }

    A.col_idx.reserve(entries.size());
    A.values.reserve(entries.size());

    for (const auto& entry : entries) {
        A.col_idx.push_back(entry.col);
        A.values.push_back(entry.value);
    }

    if (!is_valid_mat(A)) {
        throw std::runtime_error("Invalid matrix construction");
    }

    return A;
}


CSRMatrix load_matrix(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open input file");
    }

    std::string line;
    if (!std::getline(in, line)) {
        throw std::runtime_error("Empty Matrix Market file");
    }

    std::istringstream header(line);
    std::string banner, format, storage, field, symmetry;

    header >> banner >> format >> storage >> field >> symmetry;

    if (banner != MatrixMarket::HEADER
        || format != MatrixMarket::MATRIX
        || storage != MatrixMarket::COORDINATE) {
        throw std::runtime_error("Unsupported Matrix Market format");
    }
    if (field != MatrixMarket::REAL
        && field != MatrixMarket::INTEGER 
        && field != MatrixMarket::PATTERN) {
        throw std::runtime_error("Unsupported Matrix Market field");
    }
    if (symmetry != MatrixMarket::GENERAL 
        && symmetry != MatrixMarket::SYMMETRIC) {
        throw std::runtime_error("Unsupported Matrix Market symmetry");
    }

    const bool symmetric = (symmetry == MatrixMarket::SYMMETRIC);
    const bool pattern = (field == MatrixMarket::PATTERN);

    do {
        if (!std::getline(in, line)) {
            throw std::runtime_error("Missing matrix dimensions");
        }
    } while (line.empty() || line[0] == '%');

    std::istringstream dims(line);

    uint64_t rows, cols, num_non_zeroes;
    if (!(dims >> rows >> cols >> num_non_zeroes)) {
        throw std::runtime_error("Invalid matrix dimensions");
    }
    if (rows != cols) {
        throw std::runtime_error("SpTRSV requires a square matrix");
    }
    if (rows > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("Matrix dimensions exceed uint32_t");
    }

    const uint32_t n = static_cast<uint32_t>(rows);

    std::vector<Entry> entries;
    entries.reserve(static_cast<std::size_t>(num_non_zeroes));

    for (uint64_t i = 0; i < num_non_zeroes; i++) {
        uint64_t r, c;
        double value = 1.0;

        if (!(in >> r >> c)) {
            throw std::runtime_error("Failed to read matrix entry");
        }
        if (!pattern && !(in >> value)) {
            throw std::runtime_error("Failed to read matrix value");
        }
        if (r == 0 || c == 0 || r > rows || c > cols) {
            throw std::runtime_error("Matrix entry index out of bounds");
        }

        r--;
        c--;

        if (symmetric && r < c) {
            std::swap(r, c);
        }

        if (r >= c) {
            Entry entry = {
                static_cast<uint32_t>(r), 
                static_cast<uint32_t>(c), 
                value
            };
            entries.push_back(entry);
        }
    }

    std::sort(
        entries.begin(), 
        entries.end(),
        [](const Entry& a, const Entry& b) {
            if (a.row == b.row) {
                return a.col < b.col;
            }
            return a.row < b.row; }
    );

    std::vector<Entry> unique_entries;
    unique_entries.reserve(entries.size());
    
    for (const auto& entry : entries) {
        if (!unique_entries.empty() 
            && unique_entries.back().row == entry.row 
            && unique_entries.back().col == entry.col) {
            unique_entries.back().value += entry.value;
        } else {
            unique_entries.push_back(entry);
        }
    }

    if (unique_entries.size() > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("Too many CSR entries");
    }

    return build_csr_matrix(n, unique_entries);
}


void save_csr_matrix(const CSRMatrix& A, const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Cannot open output file");
    }

    const uint32_t n = A.n;
    const uint64_t num_non_zeroes = static_cast<uint64_t>(A.num_non_zeroes());

    out.write(CSR_MAGIC, sizeof(CSR_MAGIC));
    out.write(reinterpret_cast<const char*>(&n), sizeof(n));
    out.write(reinterpret_cast<const char*>(&num_non_zeroes), sizeof(num_non_zeroes));

    out.write(
        reinterpret_cast<const char*>(A.row_ptr.data()),
        A.row_ptr.size() * sizeof(uint32_t)
    );
    out.write(
        reinterpret_cast<const char*>(A.col_idx.data()),
        A.col_idx.size() * sizeof(uint32_t)
    );
    out.write(
        reinterpret_cast<const char*>(A.values.data()),
        A.values.size() * sizeof(double)
    );

    if (!out) {
        throw std::runtime_error("Failed to write CSR file: " + path);
    }
}


CSRMatrix load_csr_matrix(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open input file");
    }

    char magic[8];
    in.read(magic, sizeof(magic));

    if (!in || std::memcmp(magic, CSR_MAGIC, sizeof(magic)) != 0) {
        throw std::runtime_error("Invalid CSR file format");
    }

    uint32_t n;
    in.read(reinterpret_cast<char*>(&n), sizeof(n));

    uint64_t num_non_zeroes;
    in.read(reinterpret_cast<char*>(&num_non_zeroes), sizeof(num_non_zeroes));

    if (!in) {
        throw std::runtime_error("Failed to read CSR header");
    }

    if (num_non_zeroes > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("CSR matrix exceeds uint32_t capacity");
    }
        
    CSRMatrix A;
    A.n = n;
    A.row_ptr.resize(static_cast<std::size_t>(n) + 1);
    A.col_idx.resize(static_cast<std::size_t>(num_non_zeroes));
    A.values.resize(static_cast<std::size_t>(num_non_zeroes));

    in.read(
        reinterpret_cast<char*>(A.row_ptr.data()),
        A.row_ptr.size() * sizeof(uint32_t)
    );
    in.read(
        reinterpret_cast<char*>(A.col_idx.data()),
        A.col_idx.size() * sizeof(uint32_t)
    );
    in.read(
        reinterpret_cast<char*>(A.values.data()),
        A.values.size() * sizeof(double)
    );

    if (!in) {
        throw std::runtime_error("Failed to read CSR data");
    }

    if (!is_valid_mat(A)) {
        throw std::runtime_error("Invalid CSR matrix data");
    }

    return A;
}


bool is_valid_mat(const CSRMatrix& A) {
    for (uint32_t i = 0; i < A.n; i++) {
        const auto start = A.row_ptr[i];
        const auto end = A.row_ptr[i + 1];

        if (start == end || A.col_idx[end - 1] != i || A.values[end - 1] == 0.0) {
            return false;
        }
    }
    return true;
}


std::vector<double> matvec(
        const CSRMatrix& A, 
        const std::vector<double>& x) {
    if (x.size() != A.n) {
        throw std::invalid_argument("Vector dimension mismatch");
    }

    std::vector<double> result(A.n, 0.0);
    for (uint32_t i = 0; i < A.n; i++) {
        for (uint32_t j = A.row_ptr[i]; j < A.row_ptr[i + 1]; j++) {
            result[i] += A.values[j] * x[A.col_idx[j]];
        }
    }

    return result;
}
