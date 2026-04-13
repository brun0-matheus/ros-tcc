#ifndef _HASH_H 
#define _HASH_H 

#include <gmp.h>

#include "group.h"

void calc_hash(
        mpz_t c,
        mpz_t d,
        const group_el *X,
        const group_el *U,
        const group_el *V,
        const char *msg,
        int msg_size
);

#endif 
