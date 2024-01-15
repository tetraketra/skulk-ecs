#ifndef _ECS_H_H
#define _ECS_H_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


/*
    Each world contains many components. Worlds are on the heap.
    - `void* components`, an array of one specific component type.
    - `void (*const)(void*)`, a constructor function for that component.
    - `void (*decst)(void*)`, a destructor function for that component.
    - `size_t len`, the size of the world, which doubles at max capacity.
    - `size_t cnt`, the number of components in the world.
    - `size_t next_space`, the next free space in the world for insertion.
*/
typedef struct skulk_world_t {
    void* components;
    void (*constr)(void*);
    void (*decstr)(void*);
    size_t len;
    size_t cnt;
    size_t next_space;
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


extern void skulk_expand_universe(void);
extern skulk_universe_t _universe;
extern int64_t _uuid_counter;
extern size_t __co_defcnt;


/*
    Defines a skulk component with the given members plus those required for skulk to function. 
    Tracks the number of components defined this way to sanity-check in `skulk_component_order_end()`. 
    Must be called in the global scope!

    #### Examples:
    - `skulk_define_component(number, int value; int base;)`
    - `skulk_define_component(string, char* value; int len;)`
*/
#define skulk_define_component(name, ...)                                  \
typedef struct name { int64_t uuid; __VA_ARGS__ } name;                    \
__attribute__((constructor)) static void name##__definitely_global(void) { \
    __co_defcnt++; \
}


/*
    Starts the component order definition process. When done, finalize with `*_end()`.
    Call `skulk_component_order(component)` for each component between `*_start()` and `*_end()`.
    Must be called in the global scope without an ending semicolon!  

    Component orders defined here are accessible as the enum values `sco_##component_name`. 

    #### Macro Family:
    - `skulk_component_order_start()`
    - `skulk_component_order()`
    - `skulk_component_order_end()`
*/
#define skulk_component_order_start() enum skulk_co {


/* 
    Defines the order in which components are added to skulk's universe. 
    Call once for each component, but only between `*_start()` and `*_end()`.
    Must be called in the global scope without an ending semicolon!
*/ 
#define skulk_component_order(component) sco_##component,


/* 
    Finalizes the skulk component order.
    Must be called in global scope!
*/
#define skulk_component_order_end() __sco_final, };                      \
__attribute__((constructor)) static void scoe__definitely_global(void) { \
    for (int i = 0; i < __sco_final; i++) { skulk_expand_universe(); }   \
    assert(__co_defcnt == __sco_final && "Must call skulk_component_order() once for each component."); \
}


#endif