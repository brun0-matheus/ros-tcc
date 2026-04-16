#include "binary_ros.h"

#include <stdio.h>
#include <stdlib.h>

static void _abort(const char* msg) {
    puts(msg);
    exit(1);
}

binary_ros_data binary_ros_init(mpz_t n) {
    binary_ros_data ret;

    mpz_init_set(ret.n, n);
    ret.num_sess = mpz_sizeinbase(n, 2);
    ret.mat = (mpz_t*) calloc(2 * ret.num_sess, sizeof(mpz_t));
    ret.cnt_empty = 2*ret.num_sess;

    return ret;
}

int _real_idx(int sess, int idx) {
    return 2 * sess + idx;
}

void binary_ros_set_candidate(binary_ros_data *data, int sess, int idx, mpz_t val) {
    if(sess < 0 || sess >= data->num_sess) _abort("Invalid session identifier");
    if(idx != 0 && idx != 1) _abort("Invalid index");

    idx = _real_idx(sess, idx);
    if(data->mat[idx] != NULL) _abort("This value was already set");

    // idx^1 gives the other other value in the same session
    if(data->mat[idx^1] != NULL && mpz_cmp(data->mat[idx^1], val) == 0) _abort("Cannot repeat the value in the same session");

    mpz_init_set(data->mat[idx], val);
    data->cnt_empty--;
}

mpz_t* binary_ros_compute_coefficients(binary_ros_data *data) {
    if(data->cnt_empty != 0) _abort("Not all values were set");

    // Σ (x - val[i][0])/(val[i][1] - val[i][0]) * 2^i
    // Σ (2^i / den) * x - Σ 2^i * val[i][0] / den

    mpz_t *ret = (mpz_t*) malloc(data->num_sess * sizeof(mpz_t));

    mpz_init(data->cte_term);
    for(int i = 0; i < data->num_sess; i++) {
        mpz_init(ret[i]);

        mpz_sub(ret[i], data->mat[_real_idx(i, 1)], data->mat[_real_idx(i, 0)]);
        mpz_invert(ret[i], ret[i], data->n);
        mpz_mul_2exp(ret[i], ret[i], i);
        mpz_mod(ret[i], ret[i], data->n);

        mpz_addmul(data->cte_term, ret[i], data->mat[_real_idx(i, 0)]);
    }
    mpz_mod(data->cte_term, data->cte_term, data->n);

    return ret;
}

mpz_t* binary_ros_select_values(binary_ros_data *data, mpz_t target) {
     mpz_t tmp;
     mpz_init(tmp);

     mpz_add(tmp, target, data->cte_term);
     mpz_mod(tmp, tmp, data->n);

     mpz_t *ret = malloc(data->num_sess * sizeof(mpz_t));
     for(int i = 0; i < data->num_sess; i++) {
         int idx = _real_idx(i, mpz_tstbit(tmp, i));
         mpz_init_set(ret[i], data->mat[idx]);
     }

     mpz_clear(tmp);
     return ret;
}

void binary_ros_clear_data(binary_ros_data *data) {
    mpz_clears(data->cte_term, data->n, NULL);
    for(int i = 0; i < 2*data->num_sess; i++)
        mpz_clear(data->mat[i]);
    free(data->mat);
}

