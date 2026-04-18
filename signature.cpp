#include "signature.h"
#include "hash.h"
#include "utils.h"

Signature::Signature(
    const mpz_class &_w, 
    const GroupEl &_U,
    const GroupEl &_V,
    const GroupEl &_X,
    const Bytes &_msg
): w(_w), U(_U), V(_V), X(_X), msg(_msg) {

}

bool Signature::validate() const {
    mpz_class c, d, tmp;
    calc_hash(c, d, X, U, V, msg);

    const Group *gp = U.get_group();
    const mpz_class &n = gp->order();

    GroupEl tmp_gp(gp), rhs(gp);

    tmp_gp.mul(U, n - d);
    rhs.add(V, tmp_gp);

    tmp = c * d;
    self_mod(tmp, n);
    tmp_gp.mul(X, tmp);
    rhs.add(rhs, tmp_gp);

    tmp_gp.mul_gen(w);

    /*puts("Signature");
    printf("LHS = "); tmp_gp.print();
    printf("RHS = "); rhs.print();*/
    return tmp_gp == rhs;
}

