#include <cmath>
#include <algorithm>
#include <stdio.h>

#include "utils.h"
#include "ros.h"

using std::vector;

static void _abort(const char* msg) {
    puts(msg);
    exit(1);
}

Ros::Ros(const mpz_class &_p, int _n): p(_p), n(_n) {
    compute_pows();
}

void Ros::compute_pows() {
    mpz_class maxn = p;
    const double EPS = 1e-6;
    const double B = 1.0 / 500; // TODO: Calcular isso de forma precisa

    pows.clear();
    pows_exp.clear();

    for(int k = n-1; k > 0; k--) {
        // ceil(log(maxn, k+1))
        //int maxk = mpz_sizeinbase(maxn.get_mpz_t(), k+1);
        int maxk = std::ceil(mpz_log_base(maxn, k+1) - EPS);

        int ek = 0;
        if(k > 1) {
            // log(p, k+1)
            double logp = mpz_log_base(p, k+1);
            
            // log(p^((k-1)/k))
            double tmp = logp * ((double) (k-1))/k;

            // log(B * log(p, k+1) * p^((k-1)/k), k+1)
            double tmp2 = log_base(B * logp, k+1);
            double _ek = tmp2 + tmp;

            //printf("inp tmp2 = %lf\n", B * logp);
            //printf("logp = %lf, tmp = %lf, tmp2 = %lf, ek = %lf\n", logp, tmp, tmp2, _ek);

            ek = std::ceil(_ek-EPS);
        }

        //printf("k = %d, e_k = %d, max_k = %d\n", k, ek, maxk);
        //gmp_printf("max_number = %Zd\n", maxn);

        mpz_class cur;
        mpz_ui_pow_ui(cur.get_mpz_t(), k+1, ek);
        maxn = cur;

        for(int i = ek; i < maxk; i++) {
            pows_exp.push_back({k+1, i});
            pows.push_back(cur);
            cur *= k+1;
        }

        int cnt = maxk - ek;
        reverse(pows.end() - cnt, pows.end());
        reverse(pows_exp.end() - cnt, pows_exp.end());
    }
}

vector<int> Ros::decompose_multibase(mpz_class x) const {
    vector<int> digits;
    mpz_class dig;

    for(const auto &base: pows) {
        mpz_fdiv_qr(dig.get_mpz_t(), x.get_mpz_t(), x.get_mpz_t(), base.get_mpz_t());
        digits.push_back(dig.get_si());
    }

    return digits;
}

int Ros::num_sessions() const { return pows.size(); }

