#include <stdio.h>
#include <gmp.h>
#include <string.h>

#include "group.h"
#include "random.h"
#include "client.h"
#include "server.h"
#include "binary_ros.h"
#include "hash.h"

void show_group_stats() {
    static int lst_add = 0, lst_mul = 0, lst_mul_gen = 0, lst_mul_comb = 0;

    printf("Group add ops: %d\nGroup mul ops: %d\nGroup mul by generator ops: %d\nGroup combined mul ops: %d\n", cnt_group_add - lst_add, cnt_group_mul - lst_mul, cnt_group_mul_gen - lst_mul_gen, cnt_group_mul_comb - lst_mul_comb);

    lst_add = cnt_group_add, lst_mul = cnt_group_mul, lst_mul_gen = cnt_group_mul_gen, lst_mul_comb = cnt_group_mul_comb;
}

typedef struct {
    int i;
    client_ros_session2 client[2];
    server_session server;
    mpz_t d, c[2];
    group_el U, V;
} session_msg;

group_el X;
mpz_t private_key;

session_msg open_session_add_2ros(const group* gp, random_algo *rnd, binary_ros_data *ros, int i) {
    mpz_t c;
    session_msg ret;

    char msg[128];
    snprintf(msg, 127, "Msg %d", i);

    mpz_inits(c, ret.d, ret.c[0], ret.c[1], NULL);
    group_el_inits(gp, &ret.U, &ret.V, NULL);

    ret.server = server_commit(&ret.U, &ret.V, gp, private_key, rnd);
    client_ros_session1 *client1 = malloc(sizeof(client_ros_session1));

    *client1 = client_challenge_ros_pt1(&X, &ret.U, &ret.V, rnd);

    for(int cnt = 0; cnt < 2; cnt++) {
        ret.client[cnt] = client_challenge_ros_pt2(ret.c[cnt], ret.d, msg, strlen(msg), rnd, client1);
        binary_ros_set_candidate(ros, i, cnt, ret.c[cnt]);
    }

    ret.i = i;
    mpz_clear(c);

    return ret;
}

int main() {
    group gp;
    mpz_t n;
    binary_ros_data ros;
    session_msg *sessions;
    group_el forgV, forgU, tmp_el, blindU, blindV;
    const char *forgmsg = "Forged message";
    mpz_t *coefs, tmp, forgu, forgc, forgd, z, blindZ;

    // Initiliaziation
    random_algo *rnd = random_init();
    if(rnd == NULL) {
        puts("Could not initialize random generator");
        return 1;
    }

    group_init(&gp, 0);
    mpz_inits(n, private_key, forgc, forgd, tmp, forgu, z, blindZ, NULL);
    group_el_inits(&gp, &X, &forgV, &forgU, &tmp_el, &blindU, &blindV, NULL);
    group_order(n, &gp);

    // gen keys
    random_below(private_key, n, rnd);
    group_multiply_gen(&X, &gp, private_key);

    ros = binary_ros_init(n);
    sessions = malloc(ros.num_sess * sizeof(session_msg));
    for(int i = 0; i < ros.num_sess; i++) {
        sessions[i] = open_session_add_2ros(&gp, rnd, &ros, i);
    }

    coefs = binary_ros_compute_coefficients(&ros);
    for(int i = 0; i < ros.num_sess; i++) {
        mpz_sub(tmp, n, sessions[i].d);
        mpz_mul(tmp, tmp, coefs[i]);
        mpz_mod(tmp, tmp, n);

        group_multiply(&tmp_el, &sessions[i].U, tmp);
        group_add(&forgV, &forgV, &tmp_el);

        group_multiply(&tmp_el, &sessions[i].V, coefs[i]);
        group_add(&forgV, &forgV, &tmp_el);
    }

    random_below(forgu, n, rnd);
    group_multiply_gen(&forgU, &gp, forgu);

    calc_hash(forgc, forgd, &X, &forgU, &forgV, forgmsg, strlen(forgmsg));
    mpz_mul(tmp, forgc, forgd);
    mpz_mod(tmp, tmp, n);

    int *sel_values = binary_ros_select_values(&ros, tmp);
    for(int i = 0; i < ros.num_sess; i++) {
        int sel = sel_values[i];
        server_answer(z, sessions[i].c[sel], sessions[i].d, &sessions[i].server);

        client_sign_ros(blindZ, &blindU, &blindV, z, &sessions[i].client[sel]);

        char msg[128];
        snprintf(msg, 127, "Msg %d", i);
        char ok = verify_sign(&X, msg, strlen(msg), blindZ, &blindU, &blindV);
        if(!ok) printf("Signature %d is not ok.\n", i);
    }

    random_free(&rnd);
    mpz_clears(n, private_key, forgc, forgd, tmp, forgu, z, blindZ, NULL);
    group_el_frees(&X, &forgV, &forgU, &tmp_el, &blindU, &blindV, NULL);
    return 0;
}

