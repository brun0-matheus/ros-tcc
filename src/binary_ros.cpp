#include "binary_ros.h"
#include "utils.h"

static void _abort(const char* msg) {
    puts(msg);
    exit(1);
}

int _idx(int sess_id, int idx) {
    return 2*sess_id + idx;
}

BinaryRos::BinaryRos(const mpz_class &_n): n(_n) {
    num_sess = mpz_sizeinbase(n.get_mpz_t(), 2);
    mat.resize(num_sess*2);
}

void BinaryRos::set_value(int sess_id, int idx, const mpz_class &value) {
    mat[_idx(sess_id, idx)] = value;
}

std::vector<mpz_class> BinaryRos::compute_coefficients() {
    // Σ (x - val[i][0])/(val[i][1] - val[i][0]) * 2^i
    // den * (x - val[i][0])

    cte_term = 0;
    std::vector<mpz_class> ret(num_sess);

    for(int i = 0; i < num_sess; i++) {
        // den = 2^i / (val[i][1] - val[i][0])
        mpz_class &den = ret[i];

        den = mat[_idx(i, 1)] - mat[_idx(i, 0)];
        mpz_class_invert(den, den, n);
        mpz_mul_2exp(den.get_mpz_t(), den.get_mpz_t(), i);
        self_mod(den, n);

        mpz_addmul(cte_term.get_mpz_t(), den.get_mpz_t(), mat[_idx(i, 0)].get_mpz_t()); 
    }
    self_mod(cte_term, n);

    return ret;
}

std::vector<int> BinaryRos::select_challenges(const mpz_class &target) const {
    mpz_class real = target - cte_term;
    self_mod(real, n);

    std::vector<int> ret(num_sess);
    for(int i = 0; i < num_sess; i++)
        ret[i] = mpz_tstbit(real.get_mpz_t(), i);

    return ret;
}

int BinaryRos::num_sessions() const { return num_sess; }

int BinaryRos::num_options(int sess_id) const {
    if(sess_id < 0 || sess_id >= num_sessions())
        _abort("Invalid session id");
    return 2;
}
