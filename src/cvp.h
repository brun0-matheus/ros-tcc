#ifndef _CVP_H
#define _CVP_H

#include <vector>
#include <gmpxx.h>

// Returns the closest vector in the lattice to the target target vector
// The base vectors are the rows of the matrix
std::vector<mpz_class> solve_cvp(
    // linearized matrix, row i col j (0 indexed) is at i*n+j
    const std::vector<mpz_class> &mat,
    int n,   // size of matrix
    const std::vector<mpz_class> &target  // target vector
);

#endif
