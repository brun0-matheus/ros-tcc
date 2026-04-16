#ifndef _ROS_H
#define _ROS_H

#include <gmp.h>

typedef struct ros_data_struct {
    mpz_t n;  // group order
    long long base, num_sess;
    
    mpz_t *mat;
} ros_data;

ros_data ros_init(mpz_t n, long long base);

void ros_set_value(ros_data *data, long long sess_id, long long idx, mpz_t value);

mpz_t* ros_compute_polynomial(ros_data *data);

#endif
