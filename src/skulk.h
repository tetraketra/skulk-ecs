#ifndef _ECS_H_H
#define _ECS_H_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define uuid_t int64_t


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
    size_t len;
    size_t cnt;
    size_t next_space;
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

extern void skulk_default_pre_free(void*);
extern void* skulk_world_next_space(int world_number, size_t size_of_component);
extern void skulk_add_entity_component(int world_number, void* component, size_t size_of_component);
extern void skulk_world_init(int world_number, size_t size_of_component, void (*pre_free)(void*));
extern uuid_t skulk_get_entity_id(void);
extern void skulk_universe_expand(void);
extern skulk_universe_t skulk_universe;
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
#define skulk_define_component(name, ...)                                  \
typedef struct name { uuid_t id; __VA_ARGS__ } name;                    \
__attribute__((constructor)) static void name##__definitely_global(void) { \
    __wo_defcnt++; \
}


/* 
    Starts the world order definition process. When done, finalize with `*_end()`. 
    Call `skulk_world_order(component)` for each component between `*_start()` and `*_end()`. 
    World orders defined here are accessible as the enum values `swo_##component_name`. 
    Must be called in the global scope without an ending semicolon!   

    #### Macro Family:
    - `skulk_component_order_start()`
    - `skulk_component_order()`
    - `skulk_component_order_end()`
*/
#define skulk_world_order_start() enum skulk_wo {


/* 
    Defines the order in which worlds are added to skulk's universe. 
    Call once for each component, but only between `*_start()` and `*_end()`.
    Must be called in the global scope without an ending semicolon!
*/ 
#define skulk_world_order(component) swo_##component,


/* 
    Finalizes the skulk world order.
    Must be called in global scope!
*/
#define skulk_world_order_end() __swo_final };                      \
__attribute__((constructor)) static void scoe__definitely_global(void) { \
    for (int i = 0; i < __swo_final; i++) { skulk_universe_expand(); }   \
    assert(__wo_defcnt == __swo_final && "Must call skulk_world_order() once for each component."); \
}


#endif