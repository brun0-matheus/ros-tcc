#ifndef _GROUP_H 
#define _GROUP_H 

#include <openssl/ec.h>
#include <openssl/bn.h>
#include <gmp.h>

typedef struct group_el_struct group_el;
typedef struct group_struct group;

struct group_el_struct { 
    EC_POINT *pt;
    const group *gp;
};

struct group_struct {
    EC_GROUP *gp;
    group_el G;
    mpz_t n;
    BN_CTX *bctx;
};

extern int cnt_group_add, cnt_group_mul, cnt_group_mul_gen, cnt_group_mul_comb;

void group_init(group *gp, int option);
void group_el_init(group_el *a, const group *gp);
void group_el_inits(const group *gp, ...);

const group_el* group_generator(const group* gp);
void group_order(mpz_t out, const group* gp);

void group_copy(group_el *dst, const group_el *src); 
const group* group_from_el(const group_el* a);

void group_add(group_el *res, const group_el *a, const group_el *b);
void group_multiply(group_el *res, const group_el *a, const mpz_t k);
void group_multiply_gen(group_el *res, const group *gp, const mpz_t k);

// Compute gk * G + ek * a
void group_multiply_comb(group_el *res, const mpz_t gk, const group_el *a, const mpz_t ek);

char* group_str(const group_el *a);
void group_print(const group_el *a);

char group_equals(const group_el *a, const group_el *b);

void group_free(group *gp);
void group_el_free(group_el *a);
void group_el_frees(group_el *fst, ...);

#endif 
