#include "cvp.h"

using std::vector;

int main() {
    vector<mpz_class> mat(4);
    for(int i = 0; i < 4; i++) mat[i] = i+1;

    vector<mpz_class> target(2);
    target[0] = 6;
    target[1] = 7;

    auto res = solve_cvp(mat, 2, target);
    for(int i = 0; i < 2; i++) gmp_printf("%Zd, ", res[i]);
    puts("");

    return 0;
}
