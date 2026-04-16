#include <stdio.h>
#include <gmp.h>
#include <string.h>

#include "group.h"
#include "random.h"
#include "client.h"
#include "server.h"

void show_group_stats() {
    static int lst_add = 0, lst_mul = 0, lst_mul_gen = 0, lst_mul_comb = 0;

    printf("Group add ops: %d\nGroup mul ops: %d\nGroup mul by generator ops: %d\nGroup combined mul ops: %d\n", cnt_group_add - lst_add, cnt_group_mul - lst_mul, cnt_group_mul_gen - lst_mul_gen, cnt_group_mul_comb - lst_mul_comb);

    lst_add = cnt_group_add, lst_mul = cnt_group_mul, lst_mul_gen = cnt_group_mul_gen, lst_mul_comb = cnt_group_mul_comb;
}

int main() {
    group gp;
    group_el X, U, V, blindU, blindV;
    mpz_t c, d, z, x, n, blindZ;
    char msg[] = "Teste";
    int ret_code = 0;

    // Initiliaziation
    random_algo *rnd = random_init();
    if(rnd == NULL) {
        puts("Could not initialize random generator");
        return 1;
    }

    group_init(&gp, 0);
    mpz_inits(c, d, z, x, n, blindZ, NULL);
    group_el_inits(&gp, &X, &U, &V, &blindU, &blindV, NULL);
    
    group_order(n, &gp);

    // Key generation
    random_below(x, n, rnd);
    group_multiply_gen(&X, &gp, x);
    puts("Key generation");
    show_group_stats();

    // 1 - Server commit 
    server_session server_sess = server_commit(&U, &V, &gp, x, rnd);
    puts("\nServer commit");
    show_group_stats();

    // 2 - Client challenge 
    client_session client_sess = client_challenge(c, d, &X, &U, &V, msg, strlen(msg), rnd);
    puts("\nClient challenge");
    show_group_stats();

    // 3 - Server answer 
    server_answer(z, c, d, &server_sess);
    puts("\nServer answer");
    show_group_stats();

    // 4 - Client unblind

    if(client_sign(blindZ, &blindU, &blindV, z, &client_sess)) {
        puts("\nClient sign");
        show_group_stats();

        if(verify_sign(&X, msg, strlen(msg), blindZ, &blindU, &blindV))
            puts("\nIs the signature valid? Yes");
        else 
            puts("\nIs the signature valid? No");

        show_group_stats();
        // Tampering msg 
        msg[0] ^= 1;
        if(verify_sign(&X, msg, strlen(msg), blindZ, &blindU, &blindV))
            puts("\nIs the signature of the tampered message valid? Yes");
        else 
            puts("\nIs the signature of the tampered message valid? No");
    } else {
        puts("Server answered with an invalid signature");
        ret_code = 1;
    }

    group_free(&gp);
    group_el_frees(&X, &U, &V, &blindU, &blindV, NULL);
    client_free_session(&client_sess);
    server_free_session(&server_sess);
    random_free(&rnd);
    mpz_clears(c, d, z, x, n, blindZ, NULL);

    return ret_code;
}

