#ifndef MYRAIDENII_MACHINE_AUDIO_INTERFACE_H
#define MYRAIDENII_MACHINE_AUDIO_INTERFACE_H

#include "lib/audio/pcm/stream_interface.h"
#include "lib/types/types_interface.h"
#include "myraidenii/core/board_interface.h"

typedef struct myraidenii_machine_audio myraidenii_machine_audio;

/* The Machine bridge borrows the Board and Lib stream.  The future Machine
 * driver owns both lifetimes and must stop draining before releasing either. */
lib_status myraidenii_machine_audio_create(myraidenii_core_board *board,
    lib_audio_pcm_stream *stream, myraidenii_machine_audio **out_audio);
/* Copies available Board PCM into the bridge once, then submits ordered
 * prefixes.  A partially accepted prefix is retired exactly once; a rejected
 * suffix remains staged for a later drain. */
lib_status myraidenii_machine_audio_drain(myraidenii_machine_audio *audio);
/* Clears native pending audio first.  Staged frames remain intact if that
 * existing Lib operation fails. */
lib_status myraidenii_machine_audio_clear(myraidenii_machine_audio *audio);
void myraidenii_machine_audio_destroy(myraidenii_machine_audio *audio);

#endif
