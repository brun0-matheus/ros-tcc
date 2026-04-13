#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

#include "hash.h" 

#define HASH_SIZE 32

void finish_hash(SHA256_CTX ctx, char opt, mpz_t out, mpz_t n) {
    const int WORD = sizeof(mp_limb_t);
    const int numlimb = (HASH_SIZE + WORD - 1) / WORD;

    SHA256_Update(&ctx, &opt, 1);

    mp_limb_t *buf = mpz_limbs_write(out, numlimb);
    SHA256_Final((unsigned char *) buf, &ctx);

    mpz_limbs_finish(out, numlimb);
    mpz_mod(out, out, n);
}

void calc_hash(
        mpz_t c,
        mpz_t d,
        const group_el *X,
        const group_el *U,
        const group_el *V,
        const char *msg,
        int msg_size
) {
    SHA256_CTX ctx;
    mpz_t n;

    mpz_init(n);
    group_order(n, group_from_el(X));

    SHA256_Init(&ctx);

    char *s = group_str(X);
    SHA256_Update(&ctx, s, strlen(s));
    free(s);

    s = group_str(U);
    SHA256_Update(&ctx, s, strlen(s));
    free(s);

    s = group_str(V);
    SHA256_Update(&ctx, s, strlen(s));
    free(s);

    SHA256_Update(&ctx, msg, msg_size);

    // Calculate c
    finish_hash(ctx, 'c', c, n);
    finish_hash(ctx, 'd', d, n);

    mpz_clear(n);
}

