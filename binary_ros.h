#ifndef _BINARY_ROS_H
#define _BINARY_ROS_H

#include <gmp.h>

typedef struct binary_ros_data_struct {
    mpz_t n;
    int num_sess;

    mpz_t *mat;
    int cnt_empty;

    mpz_t cte_term;
} binary_ros_data;

binary_ros_data binary_ros_init(mpz_t n);

void binary_ros_set_candidate(binary_ros_data *data, int sess, int idx, mpz_t val);

mpz_t* binary_ros_compute_coefficients(binary_ros_data *data);

mpz_t* binary_ros_select_values(binary_ros_data *data, mpz_t target);

void binary_ros_clear_data(binary_ros_data *data);

#endif
