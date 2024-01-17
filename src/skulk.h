#ifndef _ECS_H_H
#define _ECS_H_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*
    It's just a counter :V.
*/
typedef uint64_t uuid_t;

/*
    This can be used to cast component arrays to check if there's a value.
    The first entity id is 1, so `(skulk_id_cast_t*)(thing)->id` is only 0
    if there isn't a component there.
*/
typedef struct skulk_id_cast_t {
    uuid_t id;
} skulk_id_cast_t;

/*
    Each world contains many components. Worlds are on the heap.
    - `void* components`, an array of one specific component type.
    - `void (*add_entity)(uuid_t, void*)`, a function which adds an entity.
    - `void (*del_entity)(uuid_t)`, a function which removes an entity.
    - `size_t len`, the size of the world, which doubles at max capacity.
    - `size_t cnt`, the number of components in the world.
    - `size_t next_space`, the next free space in the world for insertion.
*/
typedef struct skulk_world_t {
    void* components;
    void* next_space;
    size_t comp_size;
    size_t len;
    size_t cnt;
    void (*pre_free)(void*);
} skulk_world_t;


/*
    Each universe contains many worlds. Universes are on the stack.
    - `skulk_world_t* worlds`, an array of worlds.
    - `size_t len`, the size of the universe.
*/
typedef struct skulk_universe_t {
    skulk_world_t* worlds;
    size_t len;
} skulk_universe_t;

// sku = skulk universe
extern void         sku_expand(void);

// skw = skulk world
extern size_t       skw_ns_i(int skw_number);
extern void         skw_init(int swo_label, size_t skc_size, void (*pre_free)(void*));
extern void         skw_debug_dump(int skw_number);
extern void         skw_compress(int skw_number);
extern void         skw_del_all(int skw_number, bool chain);

// ske = skulk entity
extern uuid_t       ske_new_id(void);
extern void         ske_add(size_t n_components, ...); // &component skw_number
extern void         ske_del(uuid_t id, size_t n_components, ...); // skw_number
extern void         ske_del_all(uuid_t id);

// skec = skulk entity component
extern void         skec_add(void* component, int skw_number); // gets the id from the component
extern void         skec_del(uuid_t id, int skw_number);

// skd = skulk default
extern void         skd_pre_free(void*);

// internals
extern skulk_universe_t sku;
extern uuid_t _uuid_counter;
extern size_t __wo_defcnt;

/*
    Defines a skulk component with the given members plus those required for skulk to function.
    Tracks the number of components defined this way to sanity-check in `skulk_component_order_end()`.
    Must be called in the global scope!

    #### Examples:
    - `skulk_define_component(number, int value; int base;)`
    - `skulk_define_component(string, char* value; int len;)`
*/
#define skc_define(name, ...)                                              \
typedef struct name { uuid_t id; __VA_ARGS__ } name;                       \
__attribute__((constructor)) static void name##__definitely_global(void) { \
    __wo_defcnt++; \
}

/*
    Starts the world order definition process. When done, finalize with `*_end()`.
    Call `skulk_world_order(component)` for each component between `*_start()` and `*_end()`.
    World orders defined here are accessible as the enum values `swo_##component_name`.
    Must be called in the global scope without an ending semicolon!

    #### Macro Family:
    - `skw_order_start()`
    - `skw_order()`
    - `skw_order_end()`
*/
#define skw_order_start() enum skulk_wo {

/*
    Defines the order in which worlds are added to skulk's universe.
    Call once for each component, but only between `*_start()` and `*_end()`.
    Must be called in the global scope without an ending semicolon!
*/
#define skw_order(component) swo_##component,

/*
    Finalizes the skulk world order.
    Must be called in global scope!
*/
#define skw_order_end() __swo_final };                                   \
__attribute__((constructor)) static void scoe__definitely_global(void) { \
    for (int i = 0; i < __swo_final; i++) { sku_expand(); }              \
    assert(__wo_defcnt == __swo_final && "Must call skulk_world_order() once for each component."); \
}

/*
    Defines a skulk entity with the given components.
    Must be called in the global scope!
*/
#define ske_define(name, ...) \
    typedef struct name { __VA_ARGS__ } name;

/*
    Returns the enum value, the skulk world number, for the given skulk component.
    Do not use this in `skw_init()`!
*/
#define skw_of(skc) swo_##skc

/*
    Helper function to grab the skulk component id from a given location.
*/
#define skc_id(loc) ((skulk_id_cast_t*)(loc))->id


#endif