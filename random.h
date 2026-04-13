#ifndef _RANDOM_H 
#define _RANDOM_H 

#include <openssl/bn.h>
#include <gmp.h>

typedef struct random_struct random_algo;

random_algo* random_init();

// Generates a random integer 0 <= out < lim
//void random_below(BIGNUM *out, const BIGNUM *lim, random_algo *rnd);
void random_below(mpz_t out, const mpz_t lim, random_algo *rnd);

void random_free(random_algo **rnd);

BIGNUM* mpz_to_bignum(const mpz_t gmp_num);
void bignum_to_mpz(mpz_t gmp_num, const BIGNUM *bn);

#endif 
