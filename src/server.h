#ifndef _SERVER_H 
#define _SERVER_H 

#include <gmpxx.h>
#include <vector>

#include "random.h"
#include "group.h"

typedef struct {
    mpz_class u, v;
    bool closed;
} server_session;

class Server {
private:
    mpz_class x;
    GroupEl X;
    std::vector<server_session> sessions;

    const Group *gp;
    random_algo *rnd;
public:
    Server(const Group *_gp, random_algo *_rnd);  // generates random key

    size_t open_session(GroupEl &U, GroupEl &V);
    mpz_class finish_session(size_t sess_id, const mpz_class &c, const mpz_class &d);

    const GroupEl& pubkey() const;
};

#endif 
