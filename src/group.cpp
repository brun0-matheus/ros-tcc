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

const Group *GroupEl::default_gp = NULL;

//const int NUM_OPTS = 4;
//const int OPTS[NUM_OPTS] = {NID_secp224r1, NID_X9_62_prime256v1, NID_secp384r1, NID_secp521r1};
const int NUM_OPTS = 1;
const int OPTS[NUM_OPTS] = {NID_secp256k1};

Group::Group(int option) {
    if(option < 0 || option >= NUM_OPTS)
        _abort("Invalid group option");

    gp = EC_GROUP_new_by_curve_name(OPTS[option]);
    
    G.gp = this;
    G.pt = EC_POINT_new(gp);
    EC_POINT_copy(G.pt, EC_GROUP_get0_generator(gp));

    bctx = BN_CTX_new();

    BIGNUM *tmp = BN_new();
    EC_GROUP_get_order(gp, tmp, bctx);
    bignum_to_mpz(n.get_mpz_t(), tmp);
    BN_free(tmp);
}

Group::~Group() {
    EC_GROUP_free(gp);
    BN_CTX_free(bctx);
}

const GroupEl& Group::generator() const { return G; }
const mpz_class& Group::order() const { return n; }

GroupEl::GroupEl() {
    if(default_gp != NULL) {
        gp = default_gp;
        pt = EC_POINT_new(gp->gp);
    }
}

GroupEl::GroupEl(const Group *_gp) {
    gp = _gp;
    pt = EC_POINT_new(gp->gp);
}

GroupEl::GroupEl(const GroupEl& ot) {
    gp = ot.gp;
    pt = EC_POINT_new(gp->gp);
    EC_POINT_copy(pt, ot.pt);
}

GroupEl::~GroupEl() {
    EC_POINT_free(pt);
}

const Group* GroupEl::get_group() const { return gp; }

void GroupEl::add(const GroupEl &a, const GroupEl &b) {
    if(a.gp != b.gp) _abort("Cannot add elements from different groups");
    gp = a.gp;

    EC_POINT_add(gp->gp, pt, a.pt, b.pt, gp->bctx);
    cnt_group_add++;
}

void GroupEl::mul(const GroupEl &a, const mpz_class& k) {
    gp = a.gp;

    BIGNUM *k_bn = mpz_to_bignum(k.get_mpz_t());
    EC_POINT_mul(gp->gp, pt, NULL, a.pt, k_bn, gp->bctx);
    BN_free(k_bn);

    cnt_group_mul++;
}

void GroupEl::mul_gen(const mpz_class& k) {
    BIGNUM *k_bn = mpz_to_bignum(k.get_mpz_t());
    EC_POINT_mul(gp->gp, pt, k_bn, NULL, NULL, gp->bctx);
    BN_free(k_bn);

    cnt_group_mul_gen++;
}

void GroupEl::mul_comb(const mpz_class& gk, const GroupEl &a, const mpz_class& ek) {
    gp = a.gp;

    BIGNUM *gk_bn = mpz_to_bignum(gk.get_mpz_t());
    BIGNUM *ek_bn = mpz_to_bignum(ek.get_mpz_t());
    EC_POINT_mul(gp->gp, pt, gk_bn, a.pt, ek_bn, gp->bctx);

    BN_free(gk_bn);
    BN_free(ek_bn);
    cnt_group_mul_comb++;
}

bool GroupEl::operator==(const GroupEl &a) const {
    if(a.gp != gp) _abort("Cannot compare elements from different groups");

    return EC_POINT_cmp(gp->gp, a.pt, pt, gp->bctx)^1;
}

void GroupEl::set_default_group(const Group *gp) {
    default_gp = gp;
}

const GroupEl& GroupEl::generator() const { return gp->generator(); }
const mpz_class& GroupEl::order() const { return gp->order(); }

char* GroupEl::str() const {
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();
    EC_POINT_get_affine_coordinates(gp->gp, pt, x, y, gp->bctx);

    char *x_str = BN_bn2dec(x);
    char *y_str = BN_bn2dec(y);
    int sz = strlen(x_str) + strlen(y_str) + 5;

    char *s = (char*) malloc(sz);

    snprintf(s, sz, "(%s, %s)", x_str, y_str);

    free(x_str);
    free(y_str);
    BN_free(x);
    BN_free(y);

    return s;
}

void GroupEl::print() const {
    char *s = str();
    puts(s);
    free(s);
}

