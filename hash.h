#ifndef _HASH_H 
#define _HASH_H 

#include <gmpxx.h>

#include "group.h"
#include "utils.h"

void calc_hash(
        mpz_class &c,
        mpz_class &d,
        const GroupEl &X,
        const GroupEl &U,
        const GroupEl &V,
        const Bytes &msg
);

#endif 
