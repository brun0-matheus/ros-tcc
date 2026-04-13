#ifndef _SERVER_H 
#define _SERVER_H 

#include <gmp.h>

#include "random.h"
#include "group.h"

typedef struct server_session_struct {
    mpz_t x, u, v, n;
} server_session;

server_session server_commit(
        group_el *u,
        group_el *v,
        const group *gp,
        const mpz_t privkey,
        random_algo *rnd
);

void server_answer(
        mpz_t z,
        const mpz_t c,
        const mpz_t d,
        const server_session* session
);

void server_free_session(server_session* session);

#endif 
