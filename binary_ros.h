#ifndef _ROS_H
#define _ROS_H

#include <vector>
#include <gmpxx.h>

class BinaryRos {
private:
    mpz_class n;
    int num_sess;

public:
    std::vector<mpz_class> mat;
    mpz_class cte_term;

    BinaryRos(const mpz_class &_n);

    void set_value(int sess_id, int idx, const mpz_class &value);
    std::vector<mpz_class> compute_coefficients();
    std::vector<int> select_challenges(const mpz_class &target) const;

    int num_sessions() const;
};

#endif
