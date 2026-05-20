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
    int extra_digits;

    std::vector<std::vector<mpz_class>> mat;
    std::vector<std::vector<mpz_class>> qdif;

    std::vector<mpz_class> coefs;
    mpz_class cte_term;

    std::vector<int> base2sess;

    void compute_pows();
public:
    std::vector<mpz_class> pows;
    std::vector<int> pows_base, pows_exp;
    std::vector<int> decompose_multibase(mpz_class x) const;

    Ros(const mpz_class &p, int n, int extra_digits = 0);

    void set_value(int sess_id, int idx, const mpz_class &value);
    std::vector<mpz_class> compute_coefficients();
    std::vector<int> select_challenges(const mpz_class &target) const;

    int num_sessions() const;
    int num_options(int sess_id) const;

    void debug_print();
};

#endif
