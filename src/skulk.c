#include "skulk.h"
#include <stdio.h>

// sku = skulk universe
// skw = skulk world
// skc = skulk component
// ske = skulk entity
// skec = skulk entity component
// skd = skulk default
// ns = next space
// any place which accepts a uuid_t should use ske_new_id()
// any place that asks for skw_number can use skw_of()

/* =================== */
/* ==== INTERNALS ==== */
/* =================== */

uuid_t _uuid_counter = 1;
skulk_universe_t sku = {0};
size_t __wo_defcnt = {0};

/* ======================== */
/* ==== SKULK DEFAUILT ==== */
/* ======================== */

/*
    Define a custom pre_free per world if you want.
    Take spot in memory, convert to a specific type,
    then free things which would leak on memset.
    This default one does literally *nothing*.
*/
inline void skd_pre_free(void*) { }

/* ======================== */
/* ==== SKULK UNIVERSE ==== */
/* ======================== */

/* Expands the universe to include an additonal world. */
void sku_expand(void) {

    if ( sku.len == 0 ) {
        sku.worlds = malloc(sizeof(skulk_world_t));
    } else {
        size_t new_size = (sku.len+1)*sizeof(skulk_world_t);
        sku.worlds = realloc(sku.worlds, new_size);
    }

    memset(&sku.worlds[sku.len++], 0, sizeof(skulk_world_t));
}

/* ===================== */
/* ==== SKULK WORLD ==== */
/* ===================== */

/*
    Converts the next_space void pointer into an index.
*/
inline size_t skw_ns_i(int skw_number) {
    skulk_world_t* world = &sku.worlds[skw_number];

    return (world->next_space - world->components)/world->comp_size;
}

/*
    Initializes a skulk world. Do not use `skw_of()` here!
*/
void skw_init(int swo_label, size_t comp_size, void (*pre_free)(void*)) {
    skulk_world_t* world = &sku.worlds[swo_label]; // already zeroed

    world->comp_size = comp_size;
    world->components = calloc(1, comp_size);
    world->next_space = world->components;
    world->len = 1;
    world->pre_free = pre_free;
}

/*
    Expands a skulk world by doubling its length,
    preserving the next_space.
*/
void skw_expand(int skw_number) {
    skulk_world_t* world = &sku.worlds[skw_number];

    size_t old_ns_i = skw_ns_i(skw_number);

    world->len *= 2;
    world->components = realloc(
        world->components,
        world->len*world->comp_size
    );
    world->next_space = world->components + old_ns_i*world->comp_size;

    size_t old_size = world->len/2*world->comp_size;
    void* half_way = world->components + old_size;

    memset(half_way, 0, old_size); // set *only* the new space to 0
}

/*
    Advances the next_space in a skulk world,
    expanding said world if this would be out of bounds.
*/
void skw_ns_advance(int skw_number) {
    skulk_world_t* world = &sku.worlds[skw_number];

    while ( skc_id(world->next_space) != 0 ) {
        if ( skw_ns_i(skw_number) == world->len - 1) {
            skw_expand(skw_number);
        }
        world->next_space += world->comp_size;
    }
}

/*
    Dumps all the useful information about a skulk world.
    `total_size` and `comp_size` are in bytes,
    `len` and `cnt` are just numbers,
    and `ns_i` is the next space index.
*/
void skw_debug_dump(int skw_number) {
    skulk_world_t* world = &sku.worlds[skw_number];

    printf(
        "[DEBUG][World #%d][total_size=%ld, comp_size=%ld, len=%ld, cnt=%ld, ns_i=%ld, ", skw_number,
        world->comp_size*world->len, world->comp_size, world->len, world->cnt, skw_ns_i(skw_number)
    );

    printf("ids_at_locs=(");
    void* loc = world->components;
    for ( size_t i = 0; i < world->len; i++, loc += world->comp_size ) {
        printf("%ld:%ld, ", i, skc_id(loc));
    }

    printf(")]\n");
}

/*
    Compresses (defragments, then shortens if able) a skulk world.
    Does not preserve order.
*/
void skw_compress(int skw_number) {
    skulk_world_t* world = &sku.worlds[skw_number];

    if ( world->cnt != 0 ) { // for empty worlds after smth like skw_del_all
        void* back_iter_p = world->components + (world->len-1)*world->comp_size;

        while ( world->next_space < back_iter_p ) {
            if ( skc_id(back_iter_p) != 0 ) {
                memcpy(world->next_space, back_iter_p, world->comp_size);
                skw_ns_advance(skw_number);
                memset(back_iter_p, 0, world->comp_size);
            }

            back_iter_p -= world->comp_size;
        }
    }

    size_t new_len = 1;
    while ( new_len < (world->cnt + 1) ) {
        new_len *= 2;
    }

    size_t old_ns_i = skw_ns_i(skw_number);
    world->len = new_len;
    world->components = realloc(
        world->components,
        world->len*world->comp_size
    );
    world->next_space = world->components + old_ns_i*world->comp_size;
}

void skw_del_all(int skw_number, bool chain) {
    skulk_world_t* world = &sku.worlds[skw_number];

    void* loc = world->components;
    for ( size_t i = 0; i < world->len; i++, loc += world->comp_size ) {
        if ( skc_id(loc) != 0 ) {
            if ( chain ) {
                ske_del_all(skc_id(loc)); // double iterates over skw_number. tags: FUTURE_OPTIMIZATION
            }
            else {
                skec_del(skc_id(loc), skw_number);
            }
        }
    }
}

/* ======================== */
/* ==== SKULK ENTITY ====== */
/* ======================== */

/*
    Creates a new skulk entity id.
*/
inline uuid_t ske_new_id(void) {
    return _uuid_counter++;
}

/*
    Adds a component to a skulk entity (which doesn't actually exist as a single thing).
*/
void skec_add(void* component, int skw_number) {
    skulk_world_t* world = &sku.worlds[skw_number];

    memcpy(world->next_space, component, world->comp_size);
    skw_ns_advance(skw_number);
    world->cnt++;
}

/*
    Adds components to a skulk entity (which doesn't actually exist as a single thing).
    `n_components, (component, skw_number)...`
*/
inline void ske_add(size_t n_components, ...) {
    va_list args;
    va_start(args, n_components);

    int skw_number;
    void* component;

    for (size_t i = 0; i < n_components*2; i+=2) {
        component = va_arg(args, void*);
        skw_number = va_arg(args, int);
        skec_add(component, skw_number);
    }
}

/*
    Removes a component from a skulk entity.
    This is the most basic deletion operation.
*/
void skec_del(uuid_t id, int skw_number) {
    skulk_world_t* world = &sku.worlds[skw_number];

    void* loc = world->components;
    for (size_t i = 0; i < world->len; i++, loc += world->comp_size) {
        if (skc_id(loc) == id) {
            world->pre_free(loc);
            memset(loc, 0, world->comp_size);

            if (loc < world->next_space) {
                world->next_space = loc;
            }

            if ( --(world->cnt) == 0 ) {
                skw_compress(skw_number);
            }

            return;
        }
    }
}

/*
    Removes components from a skulk entity (which doesn't actually exist as a single thing).
    `entity_id, n_components, (skw_number)...`
*/
void ske_del(uuid_t id, size_t n_components, ...) {
    va_list args;
    va_start(args, n_components);

    int skw_number;

    for ( size_t i = 0; i < n_components; i++ ) {
        skw_number = va_arg(args, int);
        skec_del(id, skw_number);
    }
}

/*
    Removes all components from a skulk entity, no matter where they are.
    This is very slow, so don't call this unless nothing is going on.
*/
void ske_del_all(uuid_t id){
    for ( size_t i = 0; i < sku.len; i++ ) {
        skec_del(id, i);
    }
}