#ifndef _UTILS_H
#define _UTILS_H

#include <vector>
#include <gmpxx.h>

typedef std::vector<unsigned char> Bytes;

void self_mod(mpz_class &a, const mpz_class &n);
void mpz_class_invert(mpz_class &out, const mpz_class &a, const mpz_class &n);

double log_base(double x, int base);
double mpz_log_base(const mpz_class &x, int base);

#endif 
