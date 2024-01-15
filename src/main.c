#include <stdio.h>
#include "skulk.h"


skulk_define_component(number, int value; int base;)
skulk_define_component(string, char* value; int len;)

skulk_component_order_start()
skulk_component_order(string)
skulk_component_order(number)
skulk_component_order_end()

int main(void) {
    printf("%ld\n", _universe.len);
    return 0;
}





/* TODO:
- Make `init_world(.)`. Worlds are zeroed when components are defined, but we
  need the constructor and deconstructor. `init_world()` takes those.
- Make a function which takes a series of components and adds them to the ecs.
  Each component addition should call a function which inserts to the first
  available space in the world, doubling size afterwards if needed (maxed out).
*/        