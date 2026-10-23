#ifndef MYRAIDENII_MACHINE_DRIVER_INTERFACE_H
#define MYRAIDENII_MACHINE_DRIVER_INTERFACE_H

#include "emulator/machine/machine_interface.h"
#include "lib/audio/pcm/stream_interface.h"
#include "app-myraidenii/core/board_interface.h"

typedef struct myraidenii_machine myraidenii_machine;

/* The Machine borrows a fully constructed Board and takes stream ownership
 * only after it succeeds. Its owner must shut down Emulator Machine before
 * destroying this adapter or the Board. */
lib_status myraidenii_machine_create(myraidenii_core_board *board, lib_audio_pcm_stream *stream,
    myraidenii_machine **out_machine);
/* Copies the one callback descriptor used to create Emulator Machine. */
lib_status myraidenii_machine_driver_get(myraidenii_machine *machine,
    emulator_machine_driver *out_driver);
/* Enqueue mapped key releases in FIFO order; false leaves an accepted prefix. */
lib_bool myraidenii_machine_release_inputs(emulator_machine *machine);
/* A failed native-stream teardown retains Machine ownership for retry. */
lib_status myraidenii_machine_destroy(myraidenii_machine **machine);

#endif
