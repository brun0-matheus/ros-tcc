#include "client.h"
#include "hash.h"

#include <stdio.h>
#include <stdlib.h>

static void _abort(const char* msg) {
    puts(msg);
    exit(1);
}

ClientSession::ClientSession(
    Server *_server,
    const Group *_gp,
    random_algo *_rnd
): server(_server), U(_gp), V(_gp), blindU(_gp), blindV(_gp), gp(_gp), rnd(_rnd) {
    sess_id = server->open_session(U, V);
}

Signature ClientSession::finish_sign(const Bytes &msg) {
    const mpz_class &n = gp->order();
    mpz_class pi, delta, rho, epsilon, c, d, w, blindW, blindC, blindD, tmp;

    // Generate random blindness coefs 
    random_below(pi, n, rnd);
    random_below(rho, n, rnd);
    random_below(delta, n, rnd);
    random_below(epsilon, n, rnd);

    // blindU = pi*U + delta*G
    blindU.mul_comb(delta, U, pi);

    // blindV = pi*rho*V + epsilon*G 
    tmp = pi * rho;
    self_mod(tmp, n);
    blindV.mul_comb(epsilon, V, tmp);

    // blindC, blindD = H(...)
    calc_hash(blindC, blindD, server->pubkey(), blindU, blindV, msg);

    // c = blindC / pi 
    mpz_class_invert(c, pi, n);
    c *= blindC;
    self_mod(c, n);

    // d = blindD / rho 
    mpz_class_invert(d, rho, n);
    d *= blindD;
    self_mod(d, n);

    // Send challenges to server
    w = server->finish_session(sess_id, c, d);

    // Assume that the server is honest

    // Unblind the signature 
    //blindZ = rho*pi*z - delta*blindD + epsilon 

    tmp = rho*pi;
    self_mod(tmp, n);
    blindW = tmp*w - delta*blindD + epsilon;
    self_mod(blindW, n);

    /*puts("Client");
    printf("X = "); server->pubkey().print();
    printf("U = "); U.print();
    printf("V = "); V.print();
    gmp_printf("c = %Zd \nd = %Zd \nw = %Zd\n", c, d, w);
    printf("blindU = "); blindU.print();
    printf("blindV = "); blindV.print();
    gmp_printf("blindC = %Zd \nblindD = %Zd \nblindW = %Zd\n", blindC, blindD, blindW);*/

    return Signature(blindW, blindU, blindV, server->pubkey(), msg);
}


/*
client_ros_session1 client_challenge_ros_pt1(
        const group_el *pubkey,
        const group_el *U,
        const group_el *V,
        random_algo *rnd
) {
    client_ros_session1 session;
    mpz_t tmp;

    const group *gp = group_from_el(pubkey);
    if(group_from_el(U) != gp) _abort("All group ellements should be from the same group (U and X differ)");
    if(group_from_el(V) != gp) _abort("All group ellements should be from the same group (V and X differ)");
    
    mpz_inits(session.d, session.n, tmp, NULL);

    group_el_inits(gp, &session.X, &session.blindV, &session.U, &session.V, NULL);

    group_order(session.n, gp);
    group_copy(&session.X, pubkey);
    group_copy(&session.U, U);
    group_copy(&session.V, V);

    // Compute common d
    random_below(session.d, session.n, rnd);

    // Compute common blindV
    mpz_sub(tmp, session.n, session.d);
    group_multiply(&session.blindV, U, tmp);
    group_add(&session.blindV, &session.blindV, V);

    // Save inverted d 
    mpz_invert(session.d, session.d, session.n);

    session.freed = 0;

    mpz_clear(tmp);
    return session;
}

client_ros_session2 client_challenge_ros_pt2(
        mpz_t c,
        mpz_t d,
        const char* msg,
        int msg_size,
        random_algo *rnd,
        client_ros_session1* sess1
) {
    client_ros_session2 sess;
    sess.cte = sess1;
    const group *gp = group_from_el(&sess1->U);

    mpz_inits(sess.blindC, sess.blindD, sess.c, sess.u, NULL);
    group_el_init(&sess.blindU, gp);

    // copy d
    mpz_set(d, sess.cte->d);
    
    // Gen U
    random_below(sess.u, sess.cte->n, rnd);
    group_multiply_gen(&sess.blindU, gp, sess.u);

    // Compute chals
    calc_hash(sess.blindC, sess.blindD, &sess.cte->X, &sess.blindU, &sess.cte->blindV, msg, msg_size);

    // Compute c
    mpz_mul(c, sess.blindC, sess.blindD);
    mpz_mod(c, c, sess.cte->n);
    mpz_mul(c, c, d);
    mpz_mod(c, c, sess.cte->n);

    mpz_set(sess.c, c);
    return sess;
}

char client_sign_ros(
        mpz_t blindZ,
        group_el *blindU,
        group_el *blindV,
        const mpz_t z,
        const client_ros_session2 *sess
) {
    group_copy(blindU, &sess->blindU);
    group_copy(blindV, &sess->cte->blindV);

    mpz_mul(blindZ, sess->blindD, sess->u);
    mpz_sub(blindZ, z, blindZ);
    mpz_mod(blindZ, blindZ, sess->cte->n);

    return 1;  // assume the signer is honest
}

void client_free_ros(client_ros_session1 *s1, client_ros_session2 *s2) {
    if(!s1->freed) {
        mpz_clears(s1->d, s1->n, NULL);
        group_el_frees(&s1->X, &s1->blindV, &s1->U, &s1->V, NULL);
        s1->freed = 1;
    }

    mpz_clears(s2->blindC, s2->blindD, s2->c, s2->u, NULL);
    group_el_free(&s2->blindU);
}*/
