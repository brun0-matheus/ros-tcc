#include <stdlib.h>

#include "random.h"

struct random_struct {
    int nop;
};

random_algo* random_init() {
    random_algo *ret = malloc(sizeof(random_algo));
    return ret;
}

// Generates a random integer 0 <= out < lim
/*void random_below(BIGNUM *out, const BIGNUM *lim, random_algo *rnd) {
    BN_rand_range(out, lim);
}*/
void random_below(mpz_t out, const mpz_t lim, random_algo *rnd) {
    BIGNUM *out_bn = BN_new(), *lim_bn = mpz_to_bignum(lim);

    BN_rand_range(out_bn, lim_bn);
    bignum_to_mpz(out, out_bn);

    BN_free(out_bn);
    BN_free(lim_bn);
}

void random_free(random_algo **rnd) {
    if(rnd && *rnd) {
        free(*rnd);
        *rnd = NULL;
    }
}

BIGNUM* mpz_to_bignum(const mpz_t gmp_num) {
    // Determine the number of bytes needed
    size_t count;
    unsigned char *buf = mpz_export(NULL, &count, 1, 1, 1, 0, gmp_num);

    BIGNUM *bn = BN_bin2bn(buf, count, NULL);
    free(buf);

    return bn;
}

void bignum_to_mpz(mpz_t gmp_num, const BIGNUM *bn) {
    // Export BIGNUM to big-endian byte array
    int num_bytes = BN_num_bytes(bn);
    unsigned char *buf = malloc(num_bytes);
    BN_bn2bin(bn, buf);

    // Import into GMP
    mpz_import(gmp_num, num_bytes, 1, 1, 1, 0, buf);

    free(buf);
}
