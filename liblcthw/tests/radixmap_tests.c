#include "minunit.h"
#include <lcthw/radixmap.h>
//#include "../src/lcthw/radixmap.h" //just for writing
#include <time.h>
#include <inttypes.h>

static int make_random(RadixMap *map) {
    size_t i = 0;

    for (i=0; i<map->max-1; i++) {
        uint32_t key = (uint32_t) (rand() | (rand() << 16));
        check(RadixMap_add(map, key, i) == 0, "Failed to add key %u.", key);
    }

    return i;

error:
    return 0;
}

static int check_order(RadixMap * map) {
    RMElement d1, d2;
    unsigned int i = 0;

    // only signal errors if any (should not be)
    for (i=0; map->end>0 && i<map->end-1; i++) {
        d1 = map->contents[i];
        d2 = map->contents[i+1];

        if (d1.data.key > d2.data.key) {
            debug("FAIL: i=%u, key: %u, value: %u, equals max? %d\n", i, d1.data.key, d1.data.value, d2.data.key == UINT32_MAX);
            return 0;
        }
    }

    return 1;
}

static int test_search(RadixMap *map) {
    unsigned int i = 0; //in source there is no int
    RMElement *d = NULL;
    RMElement *found = NULL;

    for (i=map->end/2; i<map->end; i++) {
        d = &map->contents[i];
        found = RadixMap_find(map, d->data.key);
        check(found != NULL, "Didn't find %u at %u.", d->data.key, i);
        check(found->data.key == d->data.key, "Got the wrong result: %p:%u looking for %u at %u", found, found->data.key, d->data.key, i);
    }

    return 1;

error:
    return 0;
}

// test for big number of elements
static char *test_operations() {
    size_t N = 200;

    RadixMap *map = RadixMap_create(N);
    mu_assert(map != NULL, "Failed to make the RadixMap.");
    mu_assert(make_random(map), "Didn't make a random fake RadixMap.");

    RadixMap_sort(map);
    mu_assert(check_order(map), "Failed to properlysort the RadixMap");

    mu_assert(test_search(map), "Failed the search test.");
    mu_assert(check_order(map), "RadixMap didn't stay sorted after search.");

    while (map->end > 0) {
        RMElement *el = RadixMap_find(map, map->contents[map->end / 2].data.key);
        mu_assert(el != NULL, "Should get a result.");

        size_t old_end = map->end;

        mu_assert(RadixMap_delete(map, el) == 0, "Didn't delete it.");
        mu_assert(old_end-1 == map->end, "Wrong size after delete.");

        // test that the end is now the old value, but uint32_max so it trails off
        mu_assert(check_order(map), "RadixMap didn't stay sort after delete.");
    }

    RadixMap_destroy(map);

    return NULL;
}

void print_map(RadixMap *map) {
    for (int i=0; i<map->end; i++) {
        printf("i: %d, raw: %" PRIu64 ", key: %" PRIu32", value= %" PRIu32"\n", i, map->contents[i].raw, map->contents[i].data.key, map->contents[i].data.value);
    }
    printf("\n");

}

char *test_to_understand_radix() {
    RadixMap *map = RadixMap_create(6);
    mu_assert(map != NULL, "fail on create");
    mu_assert(map->max == 6, "wrong max");
    mu_assert(map->end == 0, "wrong end/num_elements");

    mu_assert(RadixMap_add(map, 302, 3) == 0, "failed on first add");
    //print_map(map);
    mu_assert(RadixMap_add(map, 303, 1) == 0, "failed on second add");
    //print_map(map);
    mu_assert(RadixMap_add(map, 301, 4) == 0, "failed on third add");
    //print_map(map);
    mu_assert(RadixMap_add(map, 304, 1) == 0, "failed on fourth add");
    //print_map(map);
    mu_assert(RadixMap_add(map, 305, 123) == 0, "failed on fifht add");
    //print_map(map);

    //raw je sestavlen iz value-ja in key-ja
    //ce sta value in key 4-mestna in raw 8-mesten
    //recmo da je key 123 in value 456 je raw 04560123 (najprej value nato key)
    //(to je isto predstavljen v ex35.c kjer so b biti levi od a v ab)
    //ocitno struct-i tko delajo (od odspodi navzgor)

    RadixMap_destroy(map);

    return NULL;
}

char *test_to_understand_random() {
    uint32_t rng = rand();
    //printf("rng: %" PRIu32 ", shifted: %" PRIu32 ", or rand: %" PRIu32 "\n", rng, rng<<16, rng | rng<<16);

    //rng | rng<<16 would look something like that
    //in the above code we dont use the same random number twice (we get two different ones)
    //
    //    0001000101011001 0011100000001100 (rng)
    // or 0011100000001100 0000000000000000 (shifted)
    //    ---------------------------------
    //    0011100101011101 0011100000001100 (ORd (or key in the above code))

    return NULL;
}

char *all_tests() {
    mu_suite_start();
    srand(time(NULL));

    mu_run_test(test_operations);
    mu_run_test(test_to_understand_radix); //run to understand the working
    mu_run_test(test_to_understand_random);

    //add timing
    //improve RadixMap_add by only sorting from lowest possible location of the new key till the end of the arry (not the whole array all the time)
    //improve RadixMap_add by only sorting in the range between smallest and biggest key

    return NULL;
}

RUN_TESTS(all_tests);
