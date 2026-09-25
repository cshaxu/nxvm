#include "lib/types/test.h"
#if defined(__linux__)
#define _DEFAULT_SOURCE
#endif
#include "x86/xasm32/xasm32_interface.h"
#ifdef _WIN32
#include "lib/types/win32/test.h"
#else
#include "lib/types/linux/test.h"
#endif

static void check_decode(lib_u8 *code, lib_bool flag32, lib_bool valid)
{
    char text[256], before[256];
    lib_size text_bytes = 37u, code_bytes = 19u;
    lib_memory_set(text, 0xa5, sizeof(text));
    lib_memory_copy(before, text, sizeof(text));
    lib_status status = x86_xasm32_disassemble(code, 15u, text, sizeof(text),
        &text_bytes, &code_bytes, flag32);
    if (valid) {
        lib_test_assert(status == LIB_STATUS_OK && code_bytes == 15u);
        lib_test_assert(lib_text_compare(text, "NOP     ") == 0 && text_bytes == 8u);
    } else {
        lib_test_assert(status == LIB_STATUS_UNSUPPORTED);
        lib_test_assert(text_bytes == 37u && code_bytes == 19u);
        lib_test_assert(lib_memory_compare(text, before, sizeof(text)) == 0);
    }
}

int main(void)
{
    /* The first illegal instruction byte is an inaccessible page, not padding. */
    lib_size page_bytes;
    lib_u8 *pages;
#ifdef _WIN32
    lib_win32_system_info info;
    lib_win32_dword old_protect;
    lib_win32_get_system_info(&info);
    page_bytes = info.dwPageSize;
    pages = lib_win32_virtual_alloc(LIB_NULL, page_bytes * 2u, LIB_WIN32_MEM_RESERVE | LIB_WIN32_MEM_COMMIT, LIB_WIN32_PAGE_READWRITE);
    lib_test_assert(pages != LIB_NULL);
    lib_test_assert(lib_win32_virtual_protect(pages + page_bytes, page_bytes, LIB_WIN32_PAGE_NOACCESS, &old_protect));
#else
    long page_size = lib_linux_sysconf(LIB_LINUX_SC_PAGESIZE);
    lib_test_assert(page_size > 0);
    page_bytes = (lib_size)page_size;
    pages = lib_linux_mmap(LIB_NULL, page_bytes * 2u, LIB_LINUX_PROT_READ | LIB_LINUX_PROT_WRITE,
        LIB_LINUX_MAP_PRIVATE | LIB_LINUX_MAP_ANONYMOUS, -1, 0);
    lib_test_assert(pages != LIB_LINUX_MAP_FAILED);
    lib_test_assert(lib_linux_mprotect(pages + page_bytes, page_bytes, LIB_LINUX_PROT_NONE) == 0);
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
    for (lib_i32 mode = 0; mode <= 1; ++mode) {
        for (lib_size p = 0; p < sizeof(prefixes); ++p) {
            lib_memory_set(code, prefixes[p], 15u);
            check_decode(code, mode, LIB_FALSE);
        }
        lib_memory_set(code, 0x66, 14u);
        code[14] = 0x90;
        check_decode(code, mode, LIB_TRUE);
        code[0] = 0xf1; /* No implemented handler: reject, never call LIB_NULL. */
        check_decode(code, mode, LIB_FALSE);
        for (lib_size t = 0; t < sizeof(tails) / sizeof(tails[0]); ++t) {
            lib_size length = tails[t].length;
            /* Address-size override makes SIB/disp32 meaningful in 16-bit mode. */
            for (lib_size available = 1; available < length; ++available) {
                lib_memory_set(code, 0x2e, 15u);
                if (!mode) code[0] = (t >= 2u) ? 0x67 : 0x66;
                lib_memory_copy(code + 15u - available, tails[t].bytes, available);
                check_decode(code, mode, LIB_FALSE);
            }
        }
        /* Exhaust both opcode maps at every possible remaining-byte count. */
        for (lib_size remaining = 1; remaining <= 15u; ++remaining) {
            for (lib_u32 opcode = 0; opcode < 256u; ++opcode) {
                char text[256];
                lib_size text_bytes, code_bytes;
                lib_memory_set(code, 0x2e, 15u - remaining);
                lib_memory_set(code + 15u - remaining, 0, remaining);
                code[15u - remaining] = (lib_u8)opcode;
                lib_status status = x86_xasm32_disassemble(code, 15u, text, sizeof(text),
                    &text_bytes, &code_bytes, mode);
                lib_test_assert(status == LIB_STATUS_UNSUPPORTED || (status == LIB_STATUS_OK &&
                    code_bytes > 0u && code_bytes <= 15u && text_bytes == lib_text_length(text)));
                code[15u - remaining] = 0x0f;
                if (remaining > 1u) code[16u - remaining] = (lib_u8)opcode;
                status = x86_xasm32_disassemble(code, 15u, text, sizeof(text),
                    &text_bytes, &code_bytes, mode);
                lib_test_assert(status == LIB_STATUS_UNSUPPORTED || (status == LIB_STATUS_OK &&
                    code_bytes > 0u && code_bytes <= 15u && text_bytes == lib_text_length(text)));
            }
        }
    }
#ifdef _WIN32
    lib_test_assert(lib_win32_virtual_free(pages, 0, LIB_WIN32_MEM_RELEASE));
#else
    lib_test_assert(lib_linux_munmap(pages, page_bytes * 2u) == 0);
#endif
    return 0;
}
