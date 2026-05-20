#include <stdio.h>
#include <cmath>
#include <gmp.h>
#include <string.h>
#include <chrono>
#include <cassert>
#include <iostream>

#include "binary_ros.h"
#include "group.h"
#include "utils.h"
#include "hash.h"
#include "random.h"
#include "client.h"
#include "server.h"
#include "ros.h"

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

void attack_binary(Server *server, random_algo *rnd) {
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

void attack(Server *server, random_algo *rnd, int num_dim) {
    const GroupEl &X = server->pubkey();
    const mpz_class &n = X.order();
    Ros ros(n, num_dim);

    std::vector<ClientRosSession> sess;

    for(int i = 0; i < ros.num_sessions(); i++) {
        Bytes msg = get_msg(i);
        sess.emplace_back(server, rnd, ros.num_options(i), msg);

        auto tmp = sess[i].ros_values();
        for(int j = 0; j < ros.num_options(i); j++) {
            ros.set_value(i, j, tmp[j]);
        }
    }

    std::vector<mpz_class> coefs = ros.compute_coefficients();
    /*gmp_fprintf(stderr, "coefs = (");
    for(int i = 0; i < coefs.size(); i++) {
        if(i) gmp_fprintf(stderr, ",");
        gmp_fprintf(stderr, "%Zd", coefs[i]);
    }
    gmp_fprintf(stderr, ")\n");*/

    //ros.debug_print();

    // Compute forged signature params
    for(int attempt = 0; attempt < 100; attempt++) {
        GroupEl forgV, forgU, tmp_el;
        mpz_class forgc, forgd, forgu, forgw, tmp;
        Bytes forgMsg = get_msg(-1337);

        printf("Attempt %d...\n", attempt+1);

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
        if(selected.empty()) continue;

        mpz_class test1 = 0;
        for(int i = 0; i < ros.num_sessions(); i++) {
            int opt = selected[i];

            test1 += coefs[i] * (sess[i].c[opt] * sess[i].d % n);

            Signature sig = sess[i].finish_sign(opt, tmp);
            if(!sig.validate()) {
                printf("Signature %d is invalid.\n", i);
                return;
            }

            forgw += coefs[i] * tmp;
        }

        test1 %= n;
        assert(test1 == target);

        forgw -= forgd * forgu;
        self_mod(forgw, n);

        //gmp_fprintf(stderr, "forgw, forgu, forgc, forgd = %Zd, %Zd, %Zd, %Zd\n", forgw, forgu, forgc, forgd);
        //fprintf(stderr, "X, forgU, forgV = %s, %s, %s\n", X.str(), forgU.str(), forgV.str());

        Signature forgSig(forgw, forgU, forgV, X, forgMsg);
        if(!forgSig.validate()) {
            puts("Forged signature is invalid.");
            return;
        }

        printf("All %d+1 signatures are valid. Attack was successful.\n", ros.num_sessions());
        printf("Number of attempts: %d\n", attempt+1);
        return;
    }

    puts("Decomposition failed, resample the lattices");
}

void parse_args(int argc, const char *argv[], int &opt_gp, int &num_dim) {
    if(argc != 3) {
        puts("Usage: ./main <GROUP_OPTION> <NUMBER_DIMENSION>");
        puts("GROUP_OPTION is an integer from this list:");
        for(int i = 0; i < NUM_GROUP_OPTIONS; i++) {
            printf("  %d - %s\n", i, GROUP_OPTION_NAMES[i]);
        }
        puts("NUMBER_DIMENSION can be either 0 for the original ros, or the maximum dimension for the ros with reduced dimension.");
        exit(1);
    }

    opt_gp = atoi(argv[1]);
    if(opt_gp < 0 || opt_gp >= NUM_GROUP_OPTIONS) {
        puts("Invaild group option");
        exit(1);
    }

    num_dim = atoi(argv[2]);
    if(num_dim < 0 || num_dim == 1) {
        puts("Invalid number of dimensions. It can be either 0 (original ros) or > 1");
        exit(1);
    }
}

int main(int argc, const char *argv[]) {
    int opt, num_dim;
    parse_args(argc, argv, opt, num_dim);

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

    /*
    mpz_class u, v, c, d;
    GroupEl U, V;
    const GroupEl &X = server.pubkey();
    const mpz_class &n = X.order();
    random_below(u, n, rnd);
    random_below(v, n, rnd);
    U.mul_gen(u);
    V.mul_gen(v);
    calc_hash(c, d, X, U, V, get_msg(0));
    gmp_printf("X = %s\nU = %s\nV = %s\nc = %Zd\nd = %Zd\n", X.str(), U.str(), V.str(), c, d);
    return 0;
    */

    auto t0 = high_resolution_clock::now();
    if(num_dim == 0) {
        attack_binary(&server, rnd);
    } else {
        attack(&server, rnd, num_dim);
    }
    auto t1 = high_resolution_clock::now();

    duration<double> time = t1 - t0;
    printf("Took %.2lf seconds.\n", time.count());

    random_free(&rnd);
    return 0;
}

