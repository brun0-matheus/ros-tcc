#include "server.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

static void _abort(const char* msg) {
    puts(msg);
    exit(1);
}

Server::Server(const Group *_gp, random_algo *_rnd): gp(_gp), rnd(_rnd) {
    const mpz_class& n = gp->order();
    random_below(x, n, rnd);
    X.mul_gen(x);
}

size_t Server::open_session(GroupEl &U, GroupEl &V) {
    size_t sess_id = sessions.size();
    const mpz_class& n = gp->order();

    sessions.push_back({0, 0});

    random_below(sessions[sess_id].u, n, rnd);
    random_below(sessions[sess_id].v, n, rnd);
    sessions[sess_id].closed = false;

    U.mul_gen(sessions[sess_id].u);
    V.mul_gen(sessions[sess_id].v);

    return sess_id;
}

mpz_class Server::finish_session(size_t sess_id, const mpz_class &c, const mpz_class &d) {
    if(sess_id < 0 || sess_id >= sessions.size()) 
        _abort("Invalid sesssion id");
    if(sessions[sess_id].closed) 
        _abort("Session is already closed");
    sessions[sess_id].closed = true;

    mpz_class tmp = c*d;
    self_mod(tmp, gp->order());

    mpz_class ret = sessions[sess_id].v - d * sessions[sess_id].u + tmp*x;
    self_mod(ret, gp->order());

    //mpz_clears(sessions[sess_id].v.get_mpz_t(), sessions[sess_id].u.get_mpz_t(), NULL);
    //
    
    //puts("Server");
    //gmp_printf("x = %Zd \nu = %Zd \nv = %Zd\n", x, sessions[sess_id].u, sessions[sess_id].v);
    return ret;
}

const GroupEl& Server::pubkey() const { return X; }

