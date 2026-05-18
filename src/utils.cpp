#include <cmath>

#include "utils.h"

using std::log2;

#include <iostream>
using std::cerr;
using std::endl;

void self_mod(mpz_class &a, const mpz_class &n) {
    mpz_mod(a.get_mpz_t(), a.get_mpz_t(), n.get_mpz_t());
}

void mpz_class_invert(mpz_class &out, const mpz_class &a, const mpz_class &n) {
    mpz_invert(out.get_mpz_t(), a.get_mpz_t(), n.get_mpz_t());
}

double mpz_log_base(const mpz_class &x, int base) {
    int n = mpz_size(x.get_mpz_t()) - 1;

    double logB = std::log2((double) GMP_NUMB_MASK);
    double res2 = log2((double) mpz_getlimbn(x.get_mpz_t(), n)) + n * logB;

    double res = res2 / log2((double) base);
    //cerr << "x = " << x << " base = " << base << " logB = " << logB << " res2 = " << res2 << " res = " << res << endl;

    return res;
}

double log_base(double x, int base) {
    return log2(x) / log2((double) base);
}

