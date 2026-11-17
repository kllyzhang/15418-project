#include "csr.h"

#include <cstdlib>
#include <exception>
#include <iostream>


int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <input.mtx> <output.csrbin>\n";
        exit(EXIT_FAILURE);
    }
    
    const std::string input_path = argv[1];
    const std::string output_path = argv[2];

    try {
        CSRMatrix A = load_matrix(input_path);
        save_csr_matrix(A, output_path);

        std::cout << "Successfully converted " << input_path
                  << " to " << output_path << '\n';
        std::cout << "Rows: " << A.n << '\n'
                  << "Nonzeros: " << A.num_non_zeroes() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Conversion failed: "
                  << e.what() << '\n';
        exit(EXIT_FAILURE);
    }

    return 0;
}
