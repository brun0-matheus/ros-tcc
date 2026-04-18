#ifndef _SIGNATURE_H
#define _SIGNATURE_H

#include <gmpxx.h>
#include <vector>

#include "group.h"
#include "utils.h"

class Signature {
private:
    mpz_class w;
    GroupEl U, V, X;
    Bytes msg;
public:
    Signature(
        const mpz_class &_w, 
        const GroupEl &_U,
        const GroupEl &_V,
        const GroupEl &_X,
        const Bytes &_msg
    );

    bool validate() const;
};

#endif
