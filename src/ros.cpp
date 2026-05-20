#include <cmath>
#include <algorithm>
#include <cassert>
#include <stdio.h>

#include "utils.h"
#include "ros.h"
#include "cvp.h"

using std::vector;

static void _abort(const char* msg) {
    puts(msg);
    exit(1);
}

void print_mat(const char *name, int num, const vector<vector<mpz_class>> &mat) {
    fprintf(stderr, "%s_%d = [\n", name, num);
    for(int i = 0; i < mat.size(); i++) {
        if(i)
            fprintf(stderr, ",\n");
        fprintf(stderr, "[");
        for(int j = 0; j < mat[i].size(); j++) {
            if(j)
                fprintf(stderr, ", ");
            gmp_fprintf(stderr, "%Zd", mat[i][j]);
        }
        fprintf(stderr, "]");
    }
    fprintf(stderr, "\n]\n");
}

Ros::Ros(const mpz_class &_p, int _n, int _extra): p(_p), n(_n), extra_digits(_extra) {
    compute_pows();

    mat.resize(num_sessions());
    for(int i = 0; i < num_sessions(); i++)
        mat[i].resize(num_options(i));
}

void Ros::compute_pows() {
    mpz_class maxn = p;
    const double EPS = 1e-6;
    const double B = 1.0 / 500; // TODO: Calcular isso de forma precisa

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

            ek = std::ceil(_ek-EPS) + extra_digits;
        }

        //printf("k = %d, e_k = %d, max_k = %d\n", k, ek, maxk);
        //gmp_printf("max_number = %Zd\n", maxn);

        mpz_class cur;
        mpz_ui_pow_ui(cur.get_mpz_t(), k+1, ek);
        maxn = cur;

        for(int i = ek; i < maxk; i++) {
            pows_base.push_back(k+1);
            pows_exp.push_back(i);
            pows.push_back(cur);
            cur *= k+1;
        }

        int cnt = maxk - ek;
        reverse(pows.end() - cnt, pows.end());
        reverse(pows_base.end() - cnt, pows_base.end());
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

void Ros::set_value(int sess_id, int idx, const mpz_class &value) {
    if(sess_id < 0 || sess_id >= num_sessions())
        _abort("Invalid session id");
    if(idx < 0 || idx >= num_options(sess_id))
        _abort("Invalid index for this session");

    mat[sess_id][idx] = value;
}

vector<mpz_class> Ros::compute_coefficients() {
    vector<vector<vector<mpz_class>>> lattices(num_sessions());
    vector<vector<mpz_class>> closest_vectors(num_sessions());

    coefs.resize(num_sessions());
    qdif.resize(num_sessions());

    for(int i = 0; i < num_sessions(); i++) {
        int b = pows_base[i];
        lattices[i].resize(b);

        qdif[i].resize(b-1);
        lattices[i][0].resize(b-1);
        for(int j = 0; j < b-1; j++) {
            qdif[i][j] = mat[i][j+1] - mat[i][0];
            self_mod(qdif[i][j], p);

            lattices[i][0][j] = qdif[i][j];

            lattices[i][j+1].resize(b-1);
            lattices[i][j+1][j] = p;
        }

        //print_mat("Lattice", i, lattices[i]);
    }

    cte_term = 0;
    for(int i = 0; i < num_sessions(); i++) {
        int b = pows_base[i];
        vector<mpz_class> target(b-1);
        for(int j = 0; j < b-1; j++) 
            target[j] = pows[i] * (j+1);

        closest_vectors[i] = solve_cvp(lattices[i], target);

        mpz_class mult;
        mpz_class_invert(mult, qdif[i][0], p);

        coefs[i] = (mult * closest_vectors[i][0]);
        self_mod(coefs[i], p);
        mpz_addmul(cte_term.get_mpz_t(), coefs[i].get_mpz_t(), mat[i][0].get_mpz_t()); 
    }
    self_mod(cte_term, p);

    //print_mat("Closest_vectors", 0, closest_vectors);

    vector<vector<mpz_class>> tmp;
    tmp.push_back(coefs);
    //print_mat("Coefs", 0, tmp);

    return coefs;
}

vector<int> Ros::select_challenges(const mpz_class &target) const {
    mpz_class real = target - cte_term;
    self_mod(real, p);

    vector<int> ret(num_sessions());
    for(int i = 0; i < num_sessions(); i++) {
        auto cur_digs = decompose_multibase(real);
        if(i>0 && cur_digs[i-1] != 0) 
            break;

        ret[i] = cur_digs[i];
        assert(ret[i] <= pows_base[i]);

        if(ret[i] != 0) {
            real -= (coefs[i] * qdif[i][ret[i] - 1]) % p;
        }
        //real -= (coefs[i] * mat[i][ret[i]]) % p;

        if(real <= 0) break;
    }

    if(real != 0) return {};

    mpz_class test1 = 0;
    for(int i = 0; i < num_sessions(); i++) {
        int b = ret[i];
        test1 += mat[i][b] * coefs[i];
        assert(b >= 0);
        assert(b < num_options(i));
    }
    test1 %= p;
    assert(test1 == target);

    return ret;
}

int Ros::num_sessions() const { return pows.size(); }

int Ros::num_options(int sess_id) const {
    if(sess_id < 0 || sess_id >= num_sessions())
        _abort("Invalid session id");
    return pows_base[sess_id];
}

void Ros::debug_print() {
    print_mat("Chals", 0, mat);
}
