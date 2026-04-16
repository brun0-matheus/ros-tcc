#include "ros.h"


ros_data ros_init(mpz_t n, long long base) {
    mpz_t tmp;
    ros_data data;

    mpz_inits(data.n, tmp, NULL);
    mpz_set(data.n, n);
    data->base = base;

    mpz_set_ui(tmp, base);
    data.num_sess = 1;
    while(mpz_cmp(tmp, n) < 0) {
        data.num_sess++;
        mpz_mul_ui(tmp, tmp, base);
    }

    long long mat_sz = data.num_sess * data.base;
    data.mat = (mpz_t*) calloc(mat_sz, sizeof(mpz_t));
    for(int i = 0; i < mat_sz; i++) {
        mpz_init(data.mat[i]);
    }

    mpz_clear(tmp);
    return data;
}

long long mat_idx(ros_data *data, long long sess_id, long long idx) {
    return sess_id * data->base + idx;
}

void ros_set_value(ros_data *data, long long sess_id, long long idx, mpz_t value) {
    data->mat[mat_idx(data, sess_id, idx)].set(value);
}

mpz_t* ros_compute_polynomial(ros_data *data) {
    mpz_t *ret = calloc(data->num_sess, sizeof(mpz_t));
    for(int i = 0; i < data->num_sess; i++) {
        mpz_init(ret[i]);
    }

    //  

    return ret;
}


