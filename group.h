#ifndef _GROUP_H 
#define _GROUP_H 

#include <openssl/ec.h>
#include <openssl/bn.h>
#include <gmpxx.h>

class GroupEl;
class Group;

class GroupEl {
private:
    EC_POINT *pt;
    const Group *gp;

    GroupEl(); // does not initilize, only used by Group
public:
    GroupEl(const Group *gp);
    GroupEl(const GroupEl& ot);
    ~GroupEl();

    const Group* get_group() const;

    void add(const GroupEl& a, const GroupEl& b);
    void mul(const GroupEl& a, const mpz_class& k);
    void mul_gen(const mpz_class& k);
    // Compute gk * G + ek * a
    void mul_comb(const mpz_class& gk, const GroupEl& a, const mpz_class& ek);

    char* str() const;
    void print() const;

    bool operator==(const GroupEl& a) const;

    friend class Group;
};

class Group {
private:
    EC_GROUP *gp;
    GroupEl G;
    mpz_class n;
    BN_CTX *bctx;

public:
    Group(int option);
    ~Group();

    const GroupEl& generator() const;
    const mpz_class& order() const;

    friend class GroupEl;
};


extern int cnt_group_add, cnt_group_mul, cnt_group_mul_gen, cnt_group_mul_comb;

#endif 
