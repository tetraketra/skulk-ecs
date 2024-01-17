#include <stdio.h>
#include "skulk.h"

skc_define(number, int value; int base;)
skc_define(string, char* value; int len;)

skw_order_start()
skw_order(number) // swo_number
skw_order(string) // swo_string
skw_order_end()

ske_define(named_number, number n; string s;);
ske_define(special_type_of_number, number n;);


int main(void) {

    // WORLD INITIALIZATION
    skw_init(swo_number, sizeof(number), skd_pre_free);
    skw_init(swo_string, sizeof(string), skd_pre_free);
    printf("Total worlds initialized: %ld\n", sku.len); // should be 2

    // ENTITY INITIALIZATION
    uuid_t id = ske_new_id();
    named_number ex = {
        .n = {.id = id, .value = 10, .base = 2},
        .s = {.id = id, .value = "Hello!\0", .len = 7}
    };
    ske_add(2,
        &ex.s, swo_string,
        &ex.n, swo_number  // doubles number array size to 2
    );

    uuid_t id2 = ske_new_id();
    named_number ex2 = {
        .n = {.id = id2, .value = 10, .base = 2},
        .s = {.id = id2, .value = "Hello!\0", .len = 7}
    };
    ske_add(2,
        &ex2.s, swo_string,
        &ex2.n, swo_number  // doubles number array size to 4
    );

    uuid_t id3 = ske_new_id();
    special_type_of_number jn3 = {
        .n = {.id = id3, .value = 10, .base = 2}
    };
    ske_add(1,
        &jn3.n, swo_number
    );

    // PROOF
    printf("\nProof of entity component addition: \n");
    skw_debug_dump(swo_number);
    skw_debug_dump(swo_string);

    // ENTITY DELETION
    ske_del_all(1); // v v slow
    skec_del(3, swo_number);

    // PROOF
    printf("\nProof of entity component deletion: \n");
    skw_debug_dump(swo_number);
    skw_debug_dump(swo_string);

    // WORLD DELETION
    skw_del_all(swo_number, true);

    // PROOF
    printf("\nProof of world deletion: \n");
    skw_debug_dump(swo_number);
    skw_debug_dump(swo_string);

    // // DEFRAGMENTATION
    // uuid_t id_loop;
    // special_type_of_number jn_loop = {0};
    // for (int i = 0; i < 100; i++) {
    //     id_loop = ske_new_id();
    //     jn_loop.n = (number){.id = id_loop, .value = 10, .base = 2};
    //     ske_add(1,
    //         &jn_loop.n, swo_number
    //     );
    // }
    // for (int i = 5; i < 50; i++) { // removes a large swath
    //     ske_del_all(i);
    // }

    // skw_compress(swo_number);

    // // PROOF
    // printf("\nProof of defragmented world: \n");
    // skw_debug_dump(swo_number);



    /* TODO:
        0. `skw_del_all(int skw_number, bool chain);`
        1. runtime checks to make sure you don't miss certain entity components?
    */

    return 0;
}
