#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>


/** 
 * @brief compressed sprase row matrix representation
 */
struct CSRMatrix {
    uint32_t n;
    std::vector<uint32_t> row_ptr;
    std::vector<uint32_t> col_idx;
    std::vector<double> values;
    std::size_t num_non_zeroes() const {
        return values.size();}
};

/** 
 * @brief loads an .mtx matrix file as a CSRMatrix
 * @param[in] path      path to input file location
 * @return matrix
 */
CSRMatrix load_matrix(const std::string& path);

/**
 * @brief saves a CSRMatrix object as a .csrbin file
 * @param[in] A         CSRMatrix object
 * @param[in] path      path to save location
 */
void save_csr_matrix(const CSRMatrix& A, const std::string& path);

/** 
 * @brief loads a .csrbin matrix as a CSRMatrix object
 * @param[in] path      path to input file location
 * @return matrix
 */
CSRMatrix load_csr_matrix(const std::string& path);

/** 
 * @brief validates CSRMatrix
 * @param[in] A         matrix
 * @return boolean indicator
 */
bool is_valid_mat(const CSRMatrix& A);

/** 
 * @brief multiplies a matrix in CSRMatrix form and a vector
 * @param[in] A         an N x N matrix
 * @param[in] x         a N x 1 vector
 * @return N x 1 vector of result Ax
 */
std::vector<double> matvec(
    const CSRMatrix& A, 
    const std::vector<double>& x
);
