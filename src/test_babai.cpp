#include "cvp.h"

using std::vector;

int main() {
    const int n = 2, m = 3;
    vector<vector<mpz_class>> mat(n);
    for(int i = 0; i < n; i++) {
        mat[i].resize(m);
        for(int j = 0; j < m; j++) {
            mat[i][j] = m*i+j+1;
        }
    }

    vector<mpz_class> target(m);
    target[0] = 6;
    target[1] = 7;

    auto res = solve_cvp(mat, target);
    for(int i = 0; i < m; i++) gmp_printf("%Zd, ", res[i]);
    puts("");

    return 0;
}
