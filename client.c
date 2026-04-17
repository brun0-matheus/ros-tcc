#include "client.h"
#include "hash.h"

#include <stdio.h>
#include <stdlib.h>

static void _abort(const char* msg) {
    puts(msg);
    exit(1);
}

client_session client_challenge(
        mpz_t c,
        mpz_t d,
        const group_el *pubkey,
        const group_el *U,
        const group_el *V,
        const char* msg,
        int msg_size,
        random_algo *rnd
) {
    client_session session;
    mpz_t tmp;

    const group *gp = group_from_el(pubkey);
    if(group_from_el(U) != gp) _abort("All group ellements should be from the same group (U and X differ)");
    if(group_from_el(V) != gp) _abort("All group ellements should be from the same group (V and X differ)");
    
    mpz_inits(session.blindC, session.blindD, session.pi, session.rho, session.delta, session.epsilon, session.c, session.d, session.n, tmp, NULL);

    group_el_inits(gp, &session.U, &session.V, &session.blindU, &session.blindV, &session.X, NULL);

    group_order(session.n, gp);
    group_copy(&session.X, pubkey);

    // Generate random blindness coefs 
    random_below(session.pi, session.n, rnd);
    random_below(session.rho, session.n, rnd);
    random_below(session.delta, session.n, rnd);
    random_below(session.epsilon, session.n, rnd);

    // blindU = pi*U + delta*G
    group_multiply_comb(&session.blindU, session.delta, U, session.pi);

    // blindV = pi*rho*V + epsilon*G 
    mpz_mul(tmp, session.pi, session.rho);
    mpz_mod(tmp, tmp, session.n);
    group_multiply_comb(&session.blindV, session.epsilon, V, tmp);

    // blindC, blindD = H(...)
    calc_hash(session.blindC, session.blindD, &session.X, &session.blindU, &session.blindV, msg, msg_size);

    // c = blindC / pi 
    mpz_invert(c, session.pi, session.n);
    mpz_mul(c, c, session.blindC);
    mpz_mod(c, c, session.n);

    // d = blindD / rho 
    mpz_invert(d, session.rho, session.n);
    mpz_mul(d, d, session.blindD);
    mpz_mod(d, d, session.n);

    mpz_set(session.c, c);
    mpz_set(session.d, d);
    group_copy(&session.U, U);
    group_copy(&session.V, V);

    mpz_clear(tmp);
    return session;
}

char client_sign(
        mpz_t blindZ,
        group_el *blindU,
        group_el *blindV,
        const mpz_t z,
        const client_session *session
) {
    mpz_t tmp;
    group_el tmp_el, rhs;

    const group *gp = group_from_el(&session->X);

    mpz_init(tmp);
    group_el_init(&tmp_el, gp);
    group_el_init(&rhs, gp);

    // Check server answer 
    // RHS = V - dU + cdX
    // rhs = V
    group_copy(&rhs, &session->V);

    // rhs += -dU
    mpz_sub(tmp, session->n, session->d);
    group_multiply(&tmp_el, &session->U, tmp);
    group_add(&rhs, &rhs, &tmp_el);
    
    // rhs += cdX
    mpz_mul(tmp, session->c, session->d);
    mpz_mod(tmp, tmp, session->n);
    group_multiply(&tmp_el, &session->X, tmp);
    group_add(&rhs, &rhs, &tmp_el);

    // (tmp_el) = z * G 
    group_multiply_gen(&tmp_el, gp, z);

    char retcode = 0;
    if(group_equals(&rhs, &tmp_el)) {
        // Unblind the signature 
        // blindZ = rho*pi*z - delta*blindD + epsilon 
        // blindZ = rho*pi*z
        mpz_mul(tmp, session->rho, session->pi);
        mpz_mul(blindZ, tmp, z);
        mpz_mod(blindZ, blindZ, session->n);

        // blindZ -= delta*blindD
        mpz_submul(blindZ, session->delta, session->blindD);

        // blindZ += epsilon
        mpz_add(blindZ, blindZ, session->epsilon);
                                                    
        // Only calculate mod in the end for better performance
        mpz_mod(blindZ, blindZ, session->n);

        // Set the commits 
        group_copy(blindU, &session->blindU);
        group_copy(blindV, &session->blindV);

        retcode = 1;
    } 

    mpz_clear(tmp);
    group_el_free(&tmp_el);
    group_el_free(&rhs);
    return retcode;
}

void client_free_session(client_session *session) {
    mpz_clears(session->blindC, session->blindD, session->pi, session->rho, session->delta, session->epsilon, session->c, session->d, session->n, NULL);
    group_el_frees(&session->X, &session->blindU, &session->blindV, &session->U, &session->V, NULL);
}

char verify_sign(
        const group_el *X,
        const char *msg,
        int msg_size,
        const mpz_t z,
        const group_el *U,
        const group_el *V
) {
    mpz_t c, d, n, tmp;
    group_el tmp_el, rhs;

    const group *gp = group_from_el(X);
    if(group_from_el(U) != gp) _abort("All group ellements should be from the same group (U and X differ)");
    if(group_from_el(V) != gp) _abort("All group ellements should be from the same group (V and X differ)");

    // Initialization
    mpz_inits(c, d, tmp, n, NULL);
    group_el_init(&tmp_el, gp);
    group_el_init(&rhs, gp);

    group_order(n, gp);
    
    // Calc challenges
    calc_hash(c, d, X, U, V, msg, msg_size);
    
    // RHS = V - dU + cdX
    // rhs = V
    group_copy(&rhs, V);

    // rhs += -dU
    mpz_sub(tmp, n, d);
    group_multiply(&tmp_el, U, tmp);
    group_add(&rhs, &rhs, &tmp_el);
    
    // rhs += c*d*X
    mpz_mul(tmp, c, d);
    mpz_mod(tmp, tmp, n);
    group_multiply(&tmp_el, X, tmp);
    group_add(&rhs, &rhs, &tmp_el);

    // (tmp_el) = z * G 
    group_multiply_gen(&tmp_el, gp, z);

    char retcode = group_equals(&rhs, &tmp_el);

    /*printf("Msg: %s\n", msg);
    puts("U");
    group_print(U);
    puts("V");
    group_print(V);
    puts("X");
    group_print(X);*/

    mpz_clears(c, d, tmp, n, NULL);
    group_el_free(&tmp_el);
    group_el_free(&rhs);

    return retcode;
}

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
}
