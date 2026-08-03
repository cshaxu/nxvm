# C-Library Facade

## Decision

`src/type.h` is the repository's one facade for ISO C headers and C11 atomics.
Product-neutral code includes `type.h` instead of directly including an ISO C
header. Platform integrations may additionally include their platform header.
The facade contains uppercase project aliases; direct ISO C calls outside
`type.c` are migration debt.

The existing uppercase wrapper functions are intentional and unchanged:
`LOCALTIME`, `STRCAT`, `STRCPY`, `STRTOK`, `STRCMP`, `STRLEN`, `PRINTF`,
`FPRINTF`, `SPRINTF`, `FOPEN`, `FCLOSE`, `FREAD`, `FWRITE`, `FGETS`, `MALLOC`,
`FREE`, `MEMSET`, `MEMCPY`, and `MEMCMP`.

Their underlying ISO C functions are, respectively: `localtime`, `strcat`,
`strcpy`, `strtok`, `strcmp`, `strlen`, `vfprintf`, `vsprintf`, `fopen`,
`fclose`, `fread`, `fwrite`, `fgets`, `malloc`, `free`, `memset`, `memcpy`,
and `memcmp`. `PRINTF` and `FPRINTF` both forward through `vfprintf`.

## Header Inventory

Active source uses ISO C headers `stdio.h`, `stdlib.h`, `stdarg.h`, `string.h`,
`time.h`, `stdint.h`, `stddef.h`, `stdatomic.h`, and `ctype.h`. `memory.h` is
non-standard and redundant with `string.h`; remove it when the facade is
implemented. `windows.h`, `unistd.h`, `pthread.h`, `curses.h`, and `tchar.h`
are platform dependencies and remain outside this facade.

## Function Facade

| Direct function family found | Facade spelling | Notes |
| --- | --- | --- |
| `calloc`, `malloc`, `free` | `CALLOC`, `MALLOC`, `FREE` | `MALLOC` and `FREE` already exist; add `CALLOC`. |
| `memset`, `memcpy` | `MEMSET`, `MEMCPY` | Already wrapped. |
| `strcmp`, `strlen` | `STRCMP`, `STRLEN` | Already wrapped. |
| `fopen`, `fclose`, `fread`, `fwrite` | `FOPEN`, `FCLOSE`, `FREAD`, `FWRITE` | Already wrapped. |
| `fseek`, `ftell` | `FSEEK`, `FTELL` | Add facade wrappers. |
| `fgetc`, `fputc`, `feof` | `FGETC`, `FPUTC`, `FEOF` | Add facade wrappers. |
| `snprintf` | `SNPRINTF` | Add a bounded formatting wrapper. |
| `time` | `TIME` | Add wrapper. `LOCALTIME` already exists. |
| `isspace`, `toupper` | `ISSPACE`, `TOUPPER` | Add macros that cast the argument to `unsigned char` before calling ctype. |
| `fflush` | No generic migration yet | `fflush(stdin)` occurs in the debugger and is undefined by ISO C. Replace it with an explicit platform input-flush capability, not `FFLUSH(stdin)`. |

The wrappers may be functions when ABI-safe forwarding is desired; `ISSPACE`
and `TOUPPER` are macros because their unsigned-character conversion is part of
the safety contract. The migration must not add a second behavior layer around
the existing uppercase wrappers.

## Type And Macro Facade

| Standard vocabulary found | Facade decision |
| --- | --- |
| `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`, signed counterparts | Keep in implementation-facing APIs until T107 resolves the retained `t_*` model. Do not introduce `UINT8`-style aliases because Win32 reserves/conflicts with that family. |
| `size_t`, `ptrdiff_t` | Introduce collision-safe `NTVDM64_SIZE_T` and `NTVDM64_PTRDIFF_T`; do not typedef bare `SIZE_T`, which is a Win32 type. |
| `FILE`, `time_t`, `va_list` | Introduce `NTVDM64_FILE`, `NTVDM64_TIME_T`, and `NTVDM64_VA_LIST` only where a project-owned signature needs abstraction. `FILE` itself is already uppercase and may remain in local implementation code. |
| `NULL`, `EOF`, `SEEK_*`, `stdin`, `stdout`, `stderr` | Keep standard names. They are already standard constants/objects, not project aliases. |
| `atomic_bool`, `atomic_int`, `atomic_uintptr_t`, `atomic_flag`, `atomic_*`, `memory_order_*` | Define `NTVDM64_ATOMIC_*` aliases/macros in `type.h`, preserving the exact memory-order argument. Migrate atomics in one focused task; never weaken explicit acquire/release operations. |

`BOOL` is not an ISO C type and is deliberately outside this audit. It remains
a future T107 decision because the Win32 SDK already owns that spelling.

## Active Direct-Call Inventory

Outside `type.c`, the complete direct-call set found by the audit is:
`calloc`, `free`, `malloc`, `memcpy`, `memset`, `strcmp`, `strlen`, `fopen`,
`fclose`, `fread`, `fwrite`, `fseek`, `ftell`, `fflush`, `fgetc`, `fputc`,
`feof`, `time`, `isspace`, `toupper`, and `snprintf`. They occur in
`core/machine`, core product debugger/runtime, VM composition/platform/product,
VDM minimal, CMOS/HDD, and version formatting. The full scan also found C11
atomics in core-machine, core-platform, VM composition, presentation, and
request transport.

The C language's fundamental keywords (`void`, `char`, signed/unsigned integer
forms, `float`, and `double`) are not library typedefs and remain C keywords.
The standard-library typedefs observed are `size_t`, `time_t`, `FILE`,
`va_list`, and fixed-width `intN_t`/`uintN_t` types; no active use of
`ptrdiff_t`, `clock_t`, `intptr_t`, `uintptr_t`, `wchar_t`, or C `bool` was
found. Standard objects/constants observed are `NULL`, `stdin`, `stdout`,
`stderr`, `SEEK_SET`, `SEEK_END`, and the C11 atomic type/function families.

The debugger's `fgetc` plus `feof` loop is retained during alias migration but
is a separate correctness candidate: EOF is only reliable after a failed read.
No naming task may silently rewrite that control flow.

## Migration Order

1. Add the facade declarations/macros and a focused facade smoke test.
2. Replace direct C calls and direct ISO C includes module by module, without
   changing behavior or ownership.
3. Move `fflush(stdin)` to a documented platform input-flush provider.
4. Run the full GCC, FDD prompt, and retained Console gates after each runnable
   task. Add a compile-time include scan that allows ISO C headers only in
   `type.*` and approved platform integration files.
