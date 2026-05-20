#ifndef _BINARY_ROS_H
#define _BINARY_ROS_H

#include <vector>
#include <gmpxx.h>

#include "ros.h"

class BinaryRos : public Ros {
private:
    mpz_class n;
    int num_sess;

    std::vector<mpz_class> mat;
    mpz_class cte_term;
public:
    BinaryRos(const mpz_class &_n);

    void set_value(int sess_id, int idx, const mpz_class &value);
    std::vector<mpz_class> compute_coefficients();
    std::vector<int> select_challenges(const mpz_class &target) const;

    int num_sessions() const;
    int num_options(int sess_id) const;
};

#endif
