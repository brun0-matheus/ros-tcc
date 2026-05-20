#include <fplll/fplll.h>
#include <fplll/svpcvp.h>
#include <iostream>

#include "cvp.h"

using std::vector;
using fplll::Z_NR;
using fplll::ZZ_mat;
using fplll::closest_vector;
using fplll::lll_reduction;

using std::cerr;
void print_vec(const char *name, const vector<Z_NR<mpz_t>> &v) {
    cerr << name << " = [";
    for(auto x: v) cerr << x << ", ";
    cerr << "]\n";
}

vector<mpz_class> solve_cvp(
    const vector<vector<mpz_class>> &mat,
    const vector<mpz_class> &target  // target vector
) {
    int n = mat.size(), m = mat[0].size();
    ZZ_mat<mpz_t> B;
    vector<mpz_class> ret(m);

    // copy matrix and augment it
    B.resize(n+1, m+1);
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < m; j++) 
            mpz_set(B[i][j].get_data(), mat[i][j].get_mpz_t());
        B[i][m] = 0;
    }

    // set last vector 
    mpz_class norm = 0;
    for(int i = 0; i < m; i++) {
        mpz_set(B[n][i].get_data(), target[i].get_mpz_t());
        mpz_addmul(norm.get_mpz_t(), target[i].get_mpz_t(), target[i].get_mpz_t());
    }
    mpz_sqrt(norm.get_mpz_t(), norm.get_mpz_t());
    norm++;
    mpz_set(B[n][m].get_data(), norm.get_mpz_t());

    lll_reduction(B);

    for(int i = 0; i < n+1; i++) {
        if(B[i][m] == 0) continue;

        for(int j = 0; j < m; j++) {
            if(B[i][m] > 0)
                mpz_sub(ret[j].get_mpz_t(), target[j].get_mpz_t(), B[i][j].get_data());
            else 
                mpz_add(ret[j].get_mpz_t(), target[j].get_mpz_t(), B[i][j].get_data());
        }
    }

    return ret;
}

/*
vector<mpz_class> solve_cvp(
    const vector<vector<mpz_class>> &mat,
    const vector<mpz_class> &target  // target vector
) {
    int n = mat.size(), m = mat[0].size();
    ZZ_mat<mpz_t> B, U, UT;
    vector<Z_NR<mpz_t>> t(n);
    vector<mpz_class> ret(m);

    // copy target vector
    for(int i = 0; i < m; i++) 
        mpz_set(t[i].get_data(), target[i].get_mpz_t());

    // copy matrix
    B.resize(n, m);
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < m; j++) 
            mpz_set(B[i][j].get_data(), mat[i][j].get_mpz_t());
    }

    // do babai
    lll_reduction(B);
    MatGSO<Z_NR<mpz_t>, FP_NR<mpfr_t>> M(B, U, UT, GSO_DEFAULT);
    M.update_gso();
    M.babai(t, 0, m, false);

    vector<Z_NR<mpz_t>> w = vector<Z_NR<mpz_t>>(m);
    Z_NR<mpz_t> tmp;
    for (long i = 0; i < B.get_rows(); i++) {
        for (long j = 0; j < B.get_cols(); j++) {
            tmp.mul(t[i], B[i][j]);
            w[j].add(w[j], tmp);
        }
    }

    for(int i = 0; i < m; i++) 
        mpz_set(ret[i].get_mpz_t(), w[i].get_data());
    return ret;
}*/
