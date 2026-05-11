#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

#include "hash.h" 

#define HASH_SIZE 32

void finish_hash(SHA256_CTX ctx, char opt, mpz_t out, const mpz_t n) {
    const int WORD = sizeof(mp_limb_t);
    const int numlimb = (HASH_SIZE + WORD - 1) / WORD;

    SHA256_Update(&ctx, &opt, 1);

    mp_limb_t *buf = mpz_limbs_write(out, numlimb);
    SHA256_Final((unsigned char *) buf, &ctx);

    mpz_limbs_finish(out, numlimb);
    mpz_mod(out, out, n);
}

void calc_hash(
        mpz_class &c,
        mpz_class &d,
        const GroupEl &X,
        const GroupEl &U,
        const GroupEl &V,
        const Bytes &msg
) {
    SHA256_CTX ctx;

    const mpz_class& n = X.get_group()->order();
    SHA256_Init(&ctx);

    char *s = X.str();
    SHA256_Update(&ctx, s, strlen(s));
    free(s);

    s = U.str();
    SHA256_Update(&ctx, s, strlen(s));
    free(s);

    s = V.str();
    SHA256_Update(&ctx, s, strlen(s));
    free(s);

    SHA256_Update(&ctx, msg.data(), msg.size());

    // Calculate c
    finish_hash(ctx, 'c', c.get_mpz_t(), n.get_mpz_t());
    finish_hash(ctx, 'd', d.get_mpz_t(), n.get_mpz_t());
}

