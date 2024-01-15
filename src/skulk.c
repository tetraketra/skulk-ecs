#include "skulk.h"


int64_t _uuid_counter = {0};
skulk_universe_t _universe = {0};
size_t __co_defcnt = {0}; // Component order definition counter. Uber uber private; do not touch.

void skulk_expand_universe(void) {
    if (_universe.len == 0) {
        _universe.worlds = malloc(sizeof(skulk_world_t));
    } else {
        _universe.worlds = realloc(
            _universe.worlds, 
            (_universe.len+1)*sizeof(skulk_world_t)
        );
    }

    memset(&_universe.worlds[_universe.len++], 0, sizeof(skulk_world_t));
}


// define entities using variadic macro?
