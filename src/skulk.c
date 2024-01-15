#include "skulk.h"


#include <stdio.h>

uuid_t _uuid_counter = 1;
skulk_universe_t skulk_universe = {0};
size_t __wo_defcnt = {0};

inline void skulk_default_pre_free(void*) { } // does nothing to that spot in memory

void skulk_universe_expand(void) {

    if (skulk_universe.len == 0) {
        skulk_universe.worlds = malloc(sizeof(skulk_world_t));
    } else {
        size_t new_size = (skulk_universe.len+1)*sizeof(skulk_world_t);
        skulk_universe.worlds = realloc(skulk_universe.worlds, new_size);
    }

    memset(&skulk_universe.worlds[skulk_universe.len++], 0, sizeof(skulk_world_t));
}

void skulk_world_init(int world_number, size_t size_of_component, void (*pre_free)(void*)) {
    skulk_world_t* world = &skulk_universe.worlds[world_number]; // already zeroed

    world->components = calloc(1, size_of_component);
    world->len = 1;
    world->pre_free = pre_free;
}

void skulk_world_expand(int world_number, size_t size_of_component) {
    skulk_world_t* world = &skulk_universe.worlds[world_number];

    world->len *= 2;
    world->components = realloc(
        world->components, 
        world->len*size_of_component
    );

    size_t old_size = world->len/2*size_of_component;
    void* half_way = world->components + old_size;

    memset(half_way, 0, old_size); // set *only* the new space to 0
}

inline uuid_t skulk_get_entity_id(void) {
    return _uuid_counter++;
}

inline void* skulk_world_next_space(int world_number, size_t size_of_component) { 
    skulk_world_t* world = &skulk_universe.worlds[world_number];

    return world->components + (size_of_component*world->next_space); // address of the next space
}

void skulk_add_entity_component(int world_number, void* component, size_t size_of_component) {
    skulk_world_t* world = &skulk_universe.worlds[world_number];
    void* next_space = skulk_world_next_space(world_number, size_of_component);

    memcpy(next_space, component, size_of_component);
    world->cnt++;

    while ( ((skulk_id_cast_t*)next_space)->id != 0 ) {
        if (world->next_space++ == world->len - 1) 
            skulk_world_expand(world_number, size_of_component);
        next_space = skulk_world_next_space(world_number, size_of_component);
    }
}