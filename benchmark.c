#include <assert.h>
#include <stdio.h>
#include <gmp.h>
#include <string.h>
#include <time.h>

#include "group.h"
#include "random.h"
#include "client.h"
#include "server.h"

typedef long int li;

#define NUM_ROUND 1000
#define SEC_TO_MS(sec) ((sec)*1000)
#define NS_TO_MS(ns)    ((ns)/1000000)

const char *FORMAT = 
"Time(ms): \n"
"  Server commit:     %.4lf\n"
"  Client challenge:  %.4lf\n"
"  Server answer:     %.4lf\n"
"  Client unblind:    %.4lf\n"
"  Verification:      %.4lf\n"
"  Total server:      %.4lf\n"
"  Total client:      %.4lf\n";

li get_millis() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    li ms = SEC_TO_MS((li) ts.tv_sec) + NS_TO_MS((li) ts.tv_nsec);
    return ms;
}

double A(li x) { return (double) x / NUM_ROUND; }

int main() {
    group gp;
    group_el X, U, V, blindU, blindV;
    mpz_t c, d, z, x, n, blindZ;
    char msg[] = "Teste";

    // Initiliaziation
    random_algo *rnd = random_init();
    if(rnd == NULL) {
        puts("Could not initialize random generator");
        return 1;
    }

    group_init(&gp, 0);
    mpz_inits(c, d, z, x, n, blindZ, NULL);
    group_el_inits(&gp, &X, &U, &V, &blindU, &blindV, NULL);
    
    const group_el *G = group_generator(&gp);
    group_order(n, &gp);

    li ts1=0, ts2=0, tu1=0, tu2=0, tv=0;
    for(int i = 0; i < NUM_ROUND; i++) {
        li lst, cur;

        // Key generation
        random_below(x, n, rnd);
        group_multiply(&X, G, x);
        
        // 1 - Server commit 
        lst = get_millis();
        server_session server_sess = server_commit(&U, &V, &gp, x, rnd);
        cur = get_millis();
        ts1 += cur - lst;

        // 2 - Client challenge 
        lst = get_millis();
        client_session client_sess = client_challenge(c, d, &X, &U, &V, msg, strlen(msg), rnd);
        cur = get_millis();
        tu1 += cur - lst;

        // 3 - Server answer 
        lst = get_millis();
        server_answer(z, c, d, &server_sess);
        cur = get_millis();
        ts2 += cur - lst;

        // 4 - Client unblind 
        lst = get_millis();
        int sign_ret = client_sign(blindZ, &blindU, &blindV, z, &client_sess);
        cur = get_millis();
        tu2 += cur - lst;

        assert(sign_ret);
        
        // Verification 
        lst = get_millis();
        int ver_ret = verify_sign(&X, msg, strlen(msg), blindZ, &blindU, &blindV);
        cur = get_millis();
        tv += cur - lst;

        assert(ver_ret);

        client_free_session(&client_sess);
        server_free_session(&server_sess);
    }

    printf(FORMAT,  A(ts1), A(tu1), A(ts2), A(tu2), A(tv), A(ts1 + ts2), A(tu1 + tu2));

    group_free(&gp);
    group_el_frees(&X, &U, &V, &blindU, &blindV, NULL);
    random_free(&rnd);
    mpz_clears(c, d, z, x, n, blindZ, NULL);

    return 0;
}

