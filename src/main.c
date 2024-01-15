#include <stdio.h>
#include "skulk.h"

// COMPONENT DEFINITION
skulk_define_component(number, int value; int base;)
skulk_define_component(string, char* value; int len;)

// WORLD ORDER DEFINITION
skulk_world_order_start()
skulk_world_order(number)
skulk_world_order(string)
skulk_world_order_end()

int main(void) {

    // WORLD INITIALIZATION
    skulk_world_init(swo_number, sizeof(number), skulk_default_pre_free); 
    skulk_world_init(swo_string, sizeof(string), skulk_default_pre_free);
    printf("Total worlds initialized: %ld\n", skulk_universe.len); // should be 2
    /*
        Define a custom pre_free per world if you want. 
        Take spot in memory, convert to specific type, 
        then free things which would leak on memset(0).
        The default one does literally nothing.
    */

    // ENTITY INITIALIZATION
    uuid_t id = skulk_get_entity_id();
    number ex_n = {.id = id, .value = 10, .base = 2}; 
    string ex_s = {.id = id, .value = "Hello!\0", .len = 7};
    skulk_add_entity_component(swo_number, &ex_n, sizeof(number)); // doubles number array size to 2
    skulk_add_entity_component(swo_string, &ex_s, sizeof(string));

    uuid_t id2 = skulk_get_entity_id();
    number ex_n2 = {.id = id2, .value = 10, .base = 2};
    skulk_add_entity_component(swo_number, &ex_n2, sizeof(number)); // doubles number array size to 4
    
    // PROOF OF ENTITY INITIALIZATION
    number* proof = ((number*)skulk_universe.worlds[swo_number].components);
    printf("Proof the number entities exist: \n");
    printf("id=%ld, val=%d\n", proof[0].id, proof[0].value);
    printf("id=%ld, val=%d\n", proof[1].id, proof[1].value);
    printf("id=%ld, val=%d, DOESN'T EXIST YET\n", proof[2].id, proof[2].value); 
    printf("id=%ld, val=%d, DOESN'T EXIST YET\n", proof[3].id, proof[3].value);
    /*
        `proof[4]` would trigger `-fsanitize=address` for out-of-bounds, as expected
    */ 

    // ENTITY DELETION
    /* TODO:
        Unfortunately you can't just always directly free a spot in a component array, 
        as it might contain pointers to other things. That would be a memory leak. 
        To avoid this, each world needs a `pre_free()` function
        - `skulk_del_entity_component(uuid_t id, int world_number, size_t size_of_component)` 
           to remove a component, given an id to find in the component array. This should
           call that world's `pre_free` function on that spot, then zero-out the memory,
           then `world.cnt--`, then update `world.next_space` to that location's index.
        - `skulk_del_entity(uuid_t id, size_t num_components, int world_numbers[num_components], int component_sizes[num_components])` 
           to remove an entity entirely. This just calls `skulk_del_entity_component`
           on each respective component in each respective world. Maybe the user could
           use a temporary entity struct with variable length for convenience? Not sure.
           It probably wouldn't be passed into the function though; just a usage thought.

        
    */

   // COMPONENT ARRAY DEFRAGMENTATION
   /* TODO:
        Right now, component arrays can *only* grow. We need a function which first 
        defragments (puts all components on the left and free space on the right),
        then shrinks to the smallest power of 2 that's still big enough for current
        number of components *plus one* (leaving space for the `next_space` pointer), 
        then advances the `next_space` pointer to the next free space. This operation
        might not reduce the size of the array, just defragment it.

        Sorting doesn't even remotely matter here, so just use the two-pointer solution.

        Maybe find some sort of heuristic to automatically trigger this? 
        Not sure what that would be yet tbh. Maybe track if entity insertion went a
        certain fraction of the array, then skipped something? Nah that's not
        consistent enough. Could also trigger on some pattern of deletes? Not sure.
        Oh wait, there's a `world.cnt` for this ^^;. (?). Maybe defragment whenever
        the count is below a certain fraction of the array's total size, like 25%
        or 15% or 10%? Not sure. That should be profiled and determined. Could also
        determine while the program is running using some fancy algorithm? Not sure.

        - `skulk_world_defragment(int world_number)`
   */

    return 0;
}
