#ifndef _CLIENT_H 
#define _CLIENT_H 

#include <gmp.h>

#include "group.h"
#include "random.h"

typedef struct client_session_struct {
    mpz_t blindC, blindD, pi, delta, rho, epsilon, c, d, n;
    group_el X, blindU, blindV, U, V;
} client_session;

typedef struct {
    mpz_t d, n;
    group_el X, blindV, U, V;
    char freed;
} client_ros_session1;

typedef struct {
    client_ros_session1 *cte;
    mpz_t blindC, blindD, c, u;
    group_el blindU;
} client_ros_session2;

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

client_ros_session1 client_challenge_ros_pt1(
        const group_el *pubkey,
        const group_el *U,
        const group_el *V,
        random_algo *rnd
);

client_ros_session2 client_challenge_ros_pt2(
        mpz_t c,
        mpz_t d,
        const char* msg,
        int msg_size,
        random_algo *rnd,
        client_ros_session1* sess1
);

char client_sign_ros(
        mpz_t blindZ,
        group_el *blindU,
        group_el *blindV,
        const mpz_t z,
        const client_ros_session2 *sess
);

void client_free_ros(client_ros_session1 *s1, client_ros_session2 *s2);

#endif 
