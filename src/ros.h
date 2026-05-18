#ifndef _ROS_H
#define _ROS_H

#include <vector>
#include <utility>
#include <gmpxx.h>

class Ros {
private:
    mpz_class p; // prime number
    int n;  // max base
    int num_sess;

    std::vector<mpz_class> mat;
    mpz_class cte_term;


    void compute_pows();
public:
    std::vector<mpz_class> pows;
    std::vector<std::pair<int, int>> pows_exp;
    std::vector<int> decompose_multibase(mpz_class x) const;

    Ros(const mpz_class &p, int n);

    void set_value(int sess_id, int idx, const mpz_class &value);
    std::vector<mpz_class> compute_coefficients();
    std::vector<int> select_challenges(const mpz_class &target) const;

    int num_sessions() const;
    int num_options(int sess_id) const;
};

#endif
