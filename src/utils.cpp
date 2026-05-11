#include "utils.h"

void self_mod(mpz_class &a, const mpz_class &n) {
    mpz_mod(a.get_mpz_t(), a.get_mpz_t(), n.get_mpz_t());
}

void mpz_class_invert(mpz_class &out, const mpz_class &a, const mpz_class &n) {
    mpz_invert(out.get_mpz_t(), a.get_mpz_t(), n.get_mpz_t());
}
