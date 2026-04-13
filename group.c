#include "group.h"
#include "random.h"

#include <openssl/objects.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int cnt_group_add = 0, cnt_group_mul = 0, cnt_group_mul_gen = 0, cnt_group_mul_comb = 0;

static void _abort(const char* msg) {
    puts(msg);
    exit(1);
}

void group_init(group *gp, int option) {
    gp->gp = EC_GROUP_new_by_curve_name(NID_secp224r1);
    gp->G.pt = EC_GROUP_get0_generator(gp->gp);
    gp->G.gp = gp;

    BN_CTX *ctx = BN_CTX_new();

    mpz_init(gp->n);
    BIGNUM *n = BN_new();
    EC_GROUP_get_order(gp->gp, n, ctx);
    bignum_to_mpz(gp->n, n);
    BN_free(n);

    gp->bctx = ctx;
}

void group_el_init(group_el *a, const group *gp) {
    a->pt = EC_POINT_new(gp->gp);
    a->gp = gp;
}

void group_el_inits(const group *gp, ...) {
    va_list ap;
    va_start(ap, gp);

    group_el *a;
    while((a = va_arg(ap, group_el*)) != NULL) {
        group_el_init(a, gp);
    }
    va_end(ap);
}

const group_el* group_generator(const group* gp) { return &gp->G; }

void group_order(mpz_t out, const group* gp) { mpz_set(out, gp->n); }

void group_copy(group_el *dst, const group_el *src) {
    EC_POINT_copy(dst->pt, src->pt);
    dst->gp = src->gp;
}

const group* group_from_el(const group_el* a) {
    return a->gp;
}

void group_add(group_el *res, const group_el *a, const group_el *b) {
    if(a->gp != b->gp) _abort("Cannot add elements from different groups");
    EC_POINT_add(a->gp->gp, res->pt, a->pt, b->pt, a->gp->bctx);
    res->gp = a->gp;
    cnt_group_add++;
}

void group_multiply(group_el *res, const group_el *a, const mpz_t k) {
    BIGNUM *k_bn = mpz_to_bignum(k);
    EC_POINT_mul(a->gp->gp, res->pt, NULL, a->pt, k_bn, a->gp->bctx);
    res->gp = a->gp;
    BN_free(k_bn);
    cnt_group_mul++;
}

void group_multiply_gen(group_el *res, const group *gp, const mpz_t k) {
    BIGNUM *k_bn = mpz_to_bignum(k);
    EC_POINT_mul(gp->gp, res->pt, k_bn, NULL, NULL, gp->bctx);
    res->gp = gp;
    BN_free(k_bn);
    cnt_group_mul_gen++;
}

void group_multiply_comb(group_el *res, const mpz_t gk, const group_el *a, const mpz_t ek) {
    BIGNUM *gk_bn = mpz_to_bignum(gk), *ek_bn = mpz_to_bignum(ek);
    EC_POINT_mul(a->gp->gp, res->pt, gk_bn, a->pt, ek_bn, a->gp->bctx);
    res->gp = a->gp;
    BN_free(gk_bn);
    BN_free(ek_bn);
    cnt_group_mul_comb++;
}

char group_equals(const group_el *a, const group_el *b) {
    if(a->gp != b->gp) _abort("Cannot compare elements from different groups");

    return EC_POINT_cmp(a->gp->gp, a->pt, b->pt, a->gp->bctx)^1;
}

void group_free(group *gp) {
    EC_GROUP_free(gp->gp);
    mpz_clear(gp->n);
    BN_CTX_free(gp->bctx);
}

void group_el_free(group_el *a) {
    EC_POINT_free(a->pt);
}

void group_el_frees(group_el *fst, ...) {
    if(fst == NULL) return;

    va_list ap;
    va_start(ap, fst);

    group_el_free(fst);

    group_el *a;
    while((a = va_arg(ap, group_el*)) != NULL) {
        group_el_free(a);
    }
    va_end(ap);
}

char* group_str(const group_el *a) {
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();
    EC_POINT_get_affine_coordinates(a->gp->gp, a->pt, x, y, a->gp->bctx);

    char *x_str = BN_bn2dec(x);
    char *y_str = BN_bn2dec(y);
    int sz = strlen(x_str) + strlen(y_str) + 5;

    char *s = malloc(sz);

    snprintf(s, sz, "(%s, %s)", x_str, y_str);

    OPENSSL_free(x_str);
    OPENSSL_free(y_str);
    BN_free(x);
    BN_free(y);

    return s;
}

void group_print(const group_el *a) {
    char *s = group_str(a);
    puts(s);
    free(s);
}

