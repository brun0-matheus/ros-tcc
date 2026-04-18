#ifndef _CLIENT_H 
#define _CLIENT_H 

#include <gmpxx.h>

#include "group.h"
#include "random.h"
#include "server.h"
#include "utils.h"
#include "signature.h"

class ClientSession {
private:
    Server *server;
    GroupEl U, V, blindU, blindV;
    size_t sess_id;

    const Group *gp;
    random_algo *rnd;
public:
    ClientSession(Server *server, const Group *gp, random_algo *rnd);

    Signature finish_sign(const Bytes &msg);
};


#endif 
