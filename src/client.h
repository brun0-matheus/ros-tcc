#ifndef _CLIENT_H 
#define _CLIENT_H 

#include <gmpxx.h>
#include <vector>

#include "group.h"
#include "random.h"
#include "server.h"
#include "utils.h"
#include "signature.h"

class ClientSession {
private:
    Server *server;
    GroupEl U, V, blindU, blindV;
    int sess_id;

    random_algo *rnd;
public:
    ClientSession(Server *server, random_algo *rnd);

    Signature finish_sign(const Bytes &msg);
};

class ClientRosSession {
private:
    Server *server;

    int sess_id, num_options;
    const Bytes msg;

    random_algo *rnd;
public:
    GroupEl U, V;
    GroupEl blindV;
    mpz_class d;
    std::vector<GroupEl> blindU;
    std::vector<mpz_class> u, blindC, blindD, c;

    ClientRosSession(
        Server *server,
        random_algo *rnd, 
        int num_options, 
        const Bytes &msg
    );

    std::vector<mpz_class> ros_values() const;
    Signature finish_sign(int option, mpz_class &w) const;
};

#endif 
