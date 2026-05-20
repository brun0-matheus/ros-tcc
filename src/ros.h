#ifndef _ROS_H
#define _ROS_H

#include <vector>
#include <utility>
#include <gmpxx.h>

class Ros {
public:
    virtual void set_value(int sess_id, int idx, const mpz_class &value) = 0;
    virtual std::vector<mpz_class> compute_coefficients() = 0;
    virtual std::vector<int> select_challenges(const mpz_class &target) const = 0;

    virtual int num_sessions() const = 0;
    virtual int num_options(int sess_id) const = 0;
};

#endif
