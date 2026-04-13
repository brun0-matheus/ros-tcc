#include "server.h"

#include <stdio.h>
#include <stdlib.h>

static void _abort(const char* msg) {
    puts(msg);
    exit(1);
}

server_session server_commit(group_el *u, group_el *v, const group *gp, const mpz_t privkey, random_algo *rnd) {
    server_session session;
    mpz_inits(session.x, session.u, session.v, session.n, NULL);
   
    mpz_set(session.x, privkey);
    group_order(session.n, gp);

    random_below(session.u, session.n, rnd);
    random_below(session.v, session.n, rnd);

    group_multiply_gen(u, gp, session.u);
    group_multiply_gen(v, gp, session.v);

    return session;
}

void server_answer(mpz_t z, const mpz_t c, const mpz_t d, const server_session* session) {
    if(mpz_cmp(c, session->n) >= 0 || mpz_cmp(d, session->n) >= 0) _abort("Challenges must be smaller than the group order.");
    if(mpz_sgn(c) <= 0 || mpz_sgn(d) <= 0) _abort("Challenges must be positive.");

    // z = v - d*u + c*d*x
    // z = c*d*x
    mpz_mul(z, c, d);
    mpz_mod(z, z, session->n);
    mpz_mul(z, z, session->x);

    // z -= d*u
    mpz_submul(z, d, session->u);

    // z += v
    mpz_add(z, z, session->v);
    mpz_mod(z, z, session->n);
}

void server_free_session(server_session* session) {
    mpz_clears(session->x, session->u, session->v, session->n, NULL);
}
