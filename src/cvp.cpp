#include <fplll/fplll.h>

#include "cvp.h"

using std::vector;
using fplll::Z_NR;
using fplll::ZZ_mat;
using fplll::MatGSO;
using fplll::lll_reduction;
using fplll::FP_NR;
using fplll::GSO_DEFAULT;

vector<mpz_class> solve_cvp(
    // linearized matrix, row i col j (0 indexed) is at i*n+j
    const vector<mpz_class> &mat,
    int n,   // matriz size (number of rows and cols)
    const vector<mpz_class> &target  // target vector
) {
    ZZ_mat<mpz_t> B, U, UT;
    vector<Z_NR<mpz_t>> t(n);
    vector<mpz_class> ret(n);

    // copy target vector
    for(int i = 0; i < n; i++) mpz_set(t[i].get_data(), target[i].get_mpz_t());

    // copy matrix
    B.resize(n, n);
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) 
            mpz_set(B[i][j].get_data(), mat[i*n+j].get_mpz_t());
    }

    // do babai
    lll_reduction(B);
    MatGSO<Z_NR<mpz_t>, FP_NR<mpfr_t>> M(B, U, UT, GSO_DEFAULT);
    M.update_gso();
    M.babai(t, 0, n, false);

    vector<Z_NR<mpz_t>> w = vector<Z_NR<mpz_t>>(n);
    Z_NR<mpz_t> tmp;
    for (long i = 0; i < B.get_rows(); i++) {
        for (long j = 0; j < B.get_cols(); j++) {
            tmp.mul(t[i], B[i][j]);
            w[j].add(w[j], tmp);
        }
    }

    for(int i = 0; i < n; i++) mpz_set(ret[i].get_mpz_t(), w[i].get_data());
    return ret;
}
