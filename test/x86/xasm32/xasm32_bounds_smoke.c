#if defined(__linux__)
#define _DEFAULT_SOURCE
#endif
#include "x86/xasm32/xasm32_interface.h"
#include <assert.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

static void check_decode(lib_u8 *code, int flag32, lib_bool valid)
{
    char text[256], before[256];
    lib_size text_bytes = 37u, code_bytes = 19u;
    memset(text, 0xa5, sizeof(text));
    memcpy(before, text, sizeof(text));
    lib_status status = x86_xasm32_disassemble(code, 15u, text, sizeof(text),
        &text_bytes, &code_bytes, flag32);
    if (valid) {
        assert(status == LIB_STATUS_OK && code_bytes == 15u);
        assert(strcmp(text, "NOP     ") == 0 && text_bytes == 8u);
    } else {
        assert(status == LIB_STATUS_UNSUPPORTED);
        assert(text_bytes == 37u && code_bytes == 19u);
        assert(memcmp(text, before, sizeof(text)) == 0);
    }
}

int main(void)
{
    /* The first illegal instruction byte is an inaccessible page, not padding. */
    lib_size page_bytes;
    lib_u8 *pages;
#ifdef _WIN32
    SYSTEM_INFO info;
    DWORD old_protect;
    GetSystemInfo(&info);
    page_bytes = info.dwPageSize;
    pages = VirtualAlloc(NULL, page_bytes * 2u, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    assert(pages != NULL);
    assert(VirtualProtect(pages + page_bytes, page_bytes, PAGE_NOACCESS, &old_protect));
#else
    long page_size = sysconf(_SC_PAGESIZE);
    assert(page_size > 0);
    page_bytes = (lib_size)page_size;
    pages = mmap(NULL, page_bytes * 2u, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(pages != MAP_FAILED);
    assert(mprotect(pages + page_bytes, page_bytes, PROT_NONE) == 0);
#endif
    lib_u8 *code = pages + page_bytes - 15u;
    static const lib_u8 prefixes[] = {
        0xf0, 0xf2, 0xf3, 0x2e, 0x36, 0x3e, 0x26, 0x64, 0x65, 0x66, 0x67
    };
    static const struct { lib_u8 bytes[6]; lib_size length; } tails[] = {
        {{0x0f, 0x31}, 2},                 /* extended opcode */
        {{0xb8, 1, 2, 3, 4}, 5},          /* immediate */
        {{0x8b, 0xc0}, 2},                 /* ModRM */
        {{0x8b, 0x04, 0x00}, 3},           /* SIB */
        {{0x8b, 0x85, 1, 2, 3, 4}, 6}     /* displacement */
    };
    for (int mode = 0; mode <= 1; ++mode) {
        for (lib_size p = 0; p < sizeof(prefixes); ++p) {
            memset(code, prefixes[p], 15u);
            check_decode(code, mode, LIB_FALSE);
        }
        memset(code, 0x66, 14u);
        code[14] = 0x90;
        check_decode(code, mode, LIB_TRUE);
        code[0] = 0xf1; /* No implemented handler: reject, never call NULL. */
        check_decode(code, mode, LIB_FALSE);
        for (lib_size t = 0; t < sizeof(tails) / sizeof(tails[0]); ++t) {
            lib_size length = tails[t].length;
            /* Address-size override makes SIB/disp32 meaningful in 16-bit mode. */
            for (lib_size available = 1; available < length; ++available) {
                memset(code, 0x2e, 15u);
                if (!mode) code[0] = (t >= 2u) ? 0x67 : 0x66;
                memcpy(code + 15u - available, tails[t].bytes, available);
                check_decode(code, mode, LIB_FALSE);
            }
        }
        /* Exhaust both opcode maps at every possible remaining-byte count. */
        for (lib_size remaining = 1; remaining <= 15u; ++remaining) {
            for (unsigned opcode = 0; opcode < 256u; ++opcode) {
                char text[256];
                lib_size text_bytes, code_bytes;
                memset(code, 0x2e, 15u - remaining);
                memset(code + 15u - remaining, 0, remaining);
                code[15u - remaining] = (lib_u8)opcode;
                lib_status status = x86_xasm32_disassemble(code, 15u, text, sizeof(text),
                    &text_bytes, &code_bytes, mode);
                assert(status == LIB_STATUS_UNSUPPORTED || (status == LIB_STATUS_OK &&
                    code_bytes > 0u && code_bytes <= 15u && text_bytes == strlen(text)));
                code[15u - remaining] = 0x0f;
                if (remaining > 1u) code[16u - remaining] = (lib_u8)opcode;
                status = x86_xasm32_disassemble(code, 15u, text, sizeof(text),
                    &text_bytes, &code_bytes, mode);
                assert(status == LIB_STATUS_UNSUPPORTED || (status == LIB_STATUS_OK &&
                    code_bytes > 0u && code_bytes <= 15u && text_bytes == strlen(text)));
            }
        }
    }
#ifdef _WIN32
    assert(VirtualFree(pages, 0, MEM_RELEASE));
#else
    assert(munmap(pages, page_bytes * 2u) == 0);
#endif
    return 0;
}
