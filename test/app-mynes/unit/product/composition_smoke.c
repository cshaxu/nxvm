#include <assert.h>

#include "product/composition.h"

int main(void)
{
    app_startup_config config = { .rom_path = "mynes-no-such-startup-rom.nes" };

    /* Media admission precedes session/UI construction. A missing ROM must
     * unwind the machine and driver without starting the cooked session. */
    assert(app_composition_run(&config) == 1);
    assert(app_composition_run(0) == 1);
    return 0;
}
