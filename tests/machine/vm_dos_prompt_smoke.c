#include <windows.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/machine/memory.h"
#include "vm/composition_control.h"
#include "vm/composition_live_machine.h"
#include "vm/machine/fdd.h"

#define TEXT_VIDEO_BASE 0x000b8000u
#define TEXT_VIDEO_CELLS (80u * 25u)
#define DOS_PROMPT_TIMEOUT_MILLISECONDS 3000u

static DWORD WINAPI run_full_pc(void *opaque)
{
    vm_composition_control_start((vm_composition_control_state *)opaque);
    return 0u;
}

static int has_dos_prompt(const t_ram *ram)
{
    const unsigned char *text;
    size_t cell;

    if (ram == NULL || ram->connect.pBase == 0u ||
        ram->connect.size < TEXT_VIDEO_BASE + TEXT_VIDEO_CELLS * 2u) return 0;
    text = (const unsigned char *)ram->connect.pBase + TEXT_VIDEO_BASE;
    for (cell = 0u; cell + 3u < TEXT_VIDEO_CELLS; ++cell) {
        const unsigned char drive = text[cell * 2u];
        if (isalpha((unsigned char)drive) && text[(cell + 1u) * 2u] == ':' &&
            text[(cell + 2u) * 2u] == '\\' && text[(cell + 3u) * 2u] == '>') {
            return 1;
        }
    }
    return 0;
}

static void dump_text_screen(const t_ram *ram)
{
    const unsigned char *text;
    size_t row;
    size_t column;

    if (ram == NULL || ram->connect.pBase == 0u ||
        ram->connect.size < TEXT_VIDEO_BASE + TEXT_VIDEO_CELLS * 2u) return;
    text = (const unsigned char *)ram->connect.pBase + TEXT_VIDEO_BASE;
    fputs("M5:T70:S2:SCREEN:\n", stderr);
    for (row = 0u; row < 25u; ++row) {
        for (column = 0u; column < 80u; ++column) {
            const unsigned char character = text[(row * 80u + column) * 2u];
            fputc(isprint(character) ? character : ' ', stderr);
        }
        fputc('\n', stderr);
    }
}

int main(int argc, char **argv)
{
    HANDLE thread;
    DWORD result;
    DWORD elapsed;
    int prompt_seen = 0;
    vm_composition_live_machine *session;

    if (argc != 2) return 1;
    session = (vm_composition_live_machine *)calloc(1u, sizeof(*session));
    if (session == NULL) return 1;
    vm_composition_live_machine_initialize(session);
    vm_composition_live_machine_bind_legacy(session);
    vm_composition_control_initialize(session->control, session);
    if (vm_machine_fdd_insert_for(session->fdd, argv[1]) != 0) goto fail;
    vm_composition_control_reset(session->control);
    thread = CreateThread(NULL, 0u, run_full_pc, session->control, 0u, NULL);
    if (thread == NULL) goto fail;

    for (elapsed = 0u; elapsed < DOS_PROMPT_TIMEOUT_MILLISECONDS;
         elapsed += 10u) {
        if (has_dos_prompt(session->ram)) {
            prompt_seen = 1;
            break;
        }
        Sleep(10u);
    }
    vm_composition_control_stop(session->control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    if (result != WAIT_OBJECT_0 || !prompt_seen) {
        dump_text_screen(session->ram);
        fputs("M5:T70:S2:DOS-PROMPT:TIMEOUT\n", stderr);
        goto fail;
    }
    vm_composition_control_finalize(session->control, session);
    vm_composition_live_machine_finalize(session);
    free(session);
    puts("M5:T70:S2:DOS-PROMPT:OK");
    return 0;

fail:
    vm_composition_control_stop(session->control);
    vm_composition_control_finalize(session->control, session);
    vm_composition_live_machine_finalize(session);
    free(session);
    return 1;
}
