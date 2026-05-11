#include <stdio.h>
#include <gmp.h>
#include <string.h>
#include <chrono>

#include "binary_ros.h"
#include "group.h"
#include "utils.h"
#include "hash.h"
#include "random.h"
#include "client.h"
#include "server.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration;

void show_group_stats() {
    static int lst_add = 0, lst_mul = 0, lst_mul_gen = 0, lst_mul_comb = 0;

    printf("Group add ops: %d\nGroup mul ops: %d\nGroup mul by generator ops: %d\nGroup combined mul ops: %d\n", cnt_group_add - lst_add, cnt_group_mul - lst_mul, cnt_group_mul_gen - lst_mul_gen, cnt_group_mul_comb - lst_mul_comb);

    lst_add = cnt_group_add, lst_mul = cnt_group_mul, lst_mul_gen = cnt_group_mul_gen, lst_mul_comb = cnt_group_mul_comb;
}

Bytes get_msg(int i) {
    Bytes ret(256);
    int cnt = snprintf((char*) ret.data(), 255, "Msg %d", i);
    ret.resize(cnt);

    return ret;
}

void attack(Server *server, random_algo *rnd) {
    const GroupEl &X = server->pubkey();
    const mpz_class &n = X.order();
    BinaryRos ros(n);

    std::vector<ClientRosSession> sess;

    for(int i = 0; i < ros.num_sessions(); i++) {
        Bytes msg = get_msg(i);
        sess.emplace_back(server, rnd, 2, msg);

        auto tmp = sess[i].ros_values();
        for(int j = 0; j < 2; j++) {
            ros.set_value(i, j, tmp[j]);
        }
    }

    std::vector<mpz_class> coefs = ros.compute_coefficients();

    // Compute forged signature params
    GroupEl forgV, forgU, tmp_el;
    mpz_class forgc, forgd, forgu, forgw, tmp;
    Bytes forgMsg = get_msg(-1337);

    // U
    random_below(forgu, n, rnd);
    forgU.mul_gen(forgu);

    // V
    for(int i = 0; i < ros.num_sessions(); i++) {
        // -rho * d * U
        tmp = (n - sess[i].d) * coefs[i];
        self_mod(tmp, n);
        tmp_el.mul(sess[i].U, tmp);
        forgV.add(forgV, tmp_el);
        
        // rho * V
        tmp_el.mul(sess[i].V, coefs[i]);
        forgV.add(forgV, tmp_el);
    }

    calc_hash(forgc, forgd, X, forgU, forgV, forgMsg);

    // Do ros
    mpz_class target = forgc * forgd;
    self_mod(target, n);
    std::vector<int> selected = ros.select_challenges(target);

    for(int i = 0; i < ros.num_sessions(); i++) {
        int opt = selected[i];

        Signature sig = sess[i].finish_sign(opt, tmp);
        if(!sig.validate()) {
            printf("Signature %d is invalid.\n", i);
            return;
        }

        forgw += coefs[i] * tmp;
    }

    forgw -= forgd * forgu;
    self_mod(forgw, n);

    Signature forgSig(forgw, forgU, forgV, X, forgMsg);
    if(!forgSig.validate()) {
        puts("Forged signature is invalid.");
        return;
    }

    printf("All %d+1 signatures are valid. Attack was successful.\n", ros.num_sessions());
}

int main(int argc, const char *argv[]) {
    int opt = 0;
    if(argc > 1) {
        opt = atoi(argv[1]);
    }

    Group _gp(opt);
    Group *gp = &_gp;
    GroupEl::set_default_group(gp);

    // Initiliaziation
    random_algo *rnd = random_init();
    if(rnd == NULL) {
        puts("Could not initialize random generator");
        return 1;
    }

    Server server(gp, rnd);

    auto t0 = high_resolution_clock::now();
    attack(&server, rnd);
    auto t1 = high_resolution_clock::now();

    duration<double> time = t1 - t0;
    printf("Took %.2lf seconds.\n", time.count());

    random_free(&rnd);
    return 0;
}

