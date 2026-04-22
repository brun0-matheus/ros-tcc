#include <stdio.h>
#include <gmp.h>
#include <string.h>

#include "group.h"
#include "random.h"
#include "client.h"
#include "server.h"

void show_group_stats() {
    static int lst_add = 0, lst_mul = 0, lst_mul_gen = 0, lst_mul_comb = 0;

    printf("Group add ops: %d\nGroup mul ops: %d\nGroup mul by generator ops: %d\nGroup combined mul ops: %d\n", cnt_group_add - lst_add, cnt_group_mul - lst_mul, cnt_group_mul_gen - lst_mul_gen, cnt_group_mul_comb - lst_mul_comb);

    lst_add = cnt_group_add, lst_mul = cnt_group_mul, lst_mul_gen = cnt_group_mul_gen, lst_mul_comb = cnt_group_mul_comb;
}

void test(Group *gp) {
    Bytes msg = {'T', 'e', 's', 't', 0};

    // Initiliaziation
    random_algo *rnd = random_init();
    if(rnd == NULL) {
        puts("Could not initialize random generator");
        return;
    }

    Server server(gp, rnd);
    ClientSession client(&server, rnd);
    Signature sig = client.finish_sign(msg);

    if(sig.validate()) {
        puts("The signature is valid");
    } else {
        puts("The signature is invalid");
    }

    puts("Ok");

    random_free(&rnd);

}

int main() {
    Group _gp(0);
    Group *gp = &_gp;
    GroupEl::set_default_group(gp);

    test(gp);
    return 0;
}

