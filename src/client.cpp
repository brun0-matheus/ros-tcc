#include "client.h"
#include "utils.h"
#include "hash.h"

#include <stdio.h>
#include <stdlib.h>

static void _abort(const char* msg) {
    puts(msg);
    exit(1);
}

ClientSession::ClientSession(
    Server *_server,
    random_algo *_rnd
): server(_server), rnd(_rnd) {
    sess_id = server->open_session(U, V);
}

Signature ClientSession::finish_sign(const Bytes &msg) {
    const mpz_class &n = U.get_group()->order();
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


ClientRosSession::ClientRosSession(
    Server *_server,
    random_algo *_rnd, 
    int _num_op, 
    const Bytes &_msg
): server(_server), rnd(_rnd), num_options(_num_op), msg(_msg),
    blindU(_num_op), u(_num_op), blindC(_num_op), blindD(_num_op), c(_num_op) {
    
    sess_id = server->open_session(U, V);
    const mpz_class &n = U.get_group()->order();

    random_below(d, n, rnd);

    mpz_class tmp = n - d;
    blindV.mul(U, tmp);
    blindV.add(blindV, V);

    mpz_class_invert(tmp, d, n);
    for(int i = 0; i < num_options; i++) {
        random_below(u[i], n, rnd);
        blindU[i].mul_gen(u[i]);
        calc_hash(blindC[i], blindD[i], server->pubkey(), blindU[i], blindV, msg);
        c[i] = blindC[i] * blindD[i];
        self_mod(c[i], n);
        c[i] *= tmp;
        self_mod(c[i], n);
    }
}

std::vector<mpz_class> ClientRosSession::ros_values() const {
    std::vector<mpz_class> ret(num_options);
    const mpz_class &n = U.get_group()->order();

    for(int i = 0; i < num_options; i++) {
        ret[i] = c[i] * d;
        self_mod(ret[i], n);
    }

    return ret;
}

Signature ClientRosSession::finish_sign(int option, mpz_class &w) const {
    const mpz_class &n = U.get_group()->order();
    w = server->finish_session(sess_id, c[option], d);

    mpz_class blindW = w;
    blindW -= blindD[option] * u[option];
    self_mod(blindW, n);

    return Signature(blindW, blindU[option], blindV, server->pubkey(), msg);
}
