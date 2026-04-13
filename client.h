#ifndef _CLIENT_H 
#define _CLIENT_H 

#include <gmp.h>

#include "group.h"
#include "random.h"

typedef struct client_session_struct {
    mpz_t blindC, blindD, pi, delta, rho, epsilon, c, d, n;
    group_el X, blindU, blindV, U, V;
} client_session;

client_session client_challenge(
        mpz_t c,
        mpz_t d,
        const group_el *pubkey,
        const group_el *U,
        const group_el *V,
        const char* msg,
        int msg_size,
        random_algo *rnd
);

char client_sign(
        mpz_t blindZ,
        group_el *blindU,
        group_el *blindV,
        const mpz_t z,
        const client_session *session
);

void client_free_session(client_session *session);

char verify_sign(
        const group_el *X,
        const char *msg,
        int msg_size,
        const mpz_t z,
        const group_el *U,
        const group_el *V
);

#endif 
