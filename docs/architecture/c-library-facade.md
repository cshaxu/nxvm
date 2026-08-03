# C-Library Facade

## Current Contract

This section supersedes the earlier audit record below. The operative rule is:
`type.h` owns C language aliases and ISO C vocabulary; `C_*` names scalar
typedefs, `STD_*` names ISO C types/objects/constants/atomics/wrappers, and
`WIN32_*` names project-facing Win32 vocabulary only in `platform/win32`.

Native C scalar keywords are permitted only to define the `C_*` typedefs and
inside external SDK headers. `STD_NULL` expands to `NULL`; fixed-width
`int*_t`/`uint*_t` remain unchanged. Raw `BOOL`, `SIZE_T`, and other Win32 SDK
types never leak into core, VM/VDM machine, or product code.

T111--T116 are the required closure sequence: scalar aliases; ISO C
types/objects/constants/atomics; ISO C header ownership; Win32 vocabulary;
platform input flush; then static and runtime closure. The older unprefixed
uppercase names in this document are historical audit vocabulary only.

T112 completes the second step: active project code uses `STD_SIZE_T`,
`STD_FILE`, `STD_TIME_T`, `STD_VA_LIST`, `STD_NULL`, `STD_EOF`, `STD_SEEK_*`,
standard-stream aliases, and `STD_ATOMIC_*` / `STD_MEMORY_ORDER_*`. The
atomic facade is direct token mapping; it does not insert synchronization or
alter explicit memory orders.

T113 completes the header boundary: `type.h` unconditionally owns all ISO C
headers used by active code, including `stdint.h`; `memory.h` is removed. The
`verify-c-facade-headers` build gate rejects direct ISO C includes anywhere
else in active source or tests.

T114 completes the Win32 public boundary: adapter declarations use `C_VOID`
and `WIN32_*` aliases, while native Win32 calls and callback plumbing remain
private implementation details below `platform/win32`.

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

`type.h` owns active ISO C headers `stdio.h`, `stdlib.h`, `stdarg.h`,
`stdbool.h`, `string.h`, `time.h`, `stdint.h`, `stddef.h`, `stdatomic.h`, and
`ctype.h`. `memory.h` is non-standard and has been removed. `windows.h`,
`unistd.h`, `pthread.h`, `curses.h`, and `tchar.h` are platform dependencies
and remain outside this facade.

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

## Revised Vocabulary Decision

The previous unprefixed-uppercase facade is superseded for future adoption.
The project uses three explicit domains: `C_*` for C language scalar typedefs,
`STD_*` for ISO C library vocabulary, and `WIN32_*` for Win32-only adapters.

`C_VOID`, `C_CHAR`, `C_INT`, `C_FLOAT`, and related scalar spellings are
typedef aliases, never macro substitutions for C keywords. The complete scalar
set is `C_VOID`, `C_CHAR`, `C_SCHAR`, `C_UCHAR`, `C_SHORT`, `C_USHORT`,
`C_INT`, `C_UINT`, `C_LONG`, `C_ULONG`, `C_LLONG`, `C_ULLONG`, `C_FLOAT`, and
`C_DOUBLE`. C11 Boolean vocabulary is `STD_BOOL`, based on `_Bool`; `BOOL`
does not enter project-neutral code.

All existing wrappers are renamed by a dedicated compatibility task to
`STD_*`: `STD_LOCALTIME`, `STD_STRCAT`, `STD_STRCPY`, `STD_STRTOK`,
`STD_STRCMP`, `STD_STRLEN`, `STD_PRINTF`, `STD_FPRINTF`, `STD_SPRINTF`,
`STD_FOPEN`, `STD_FCLOSE`, `STD_FREAD`, `STD_FWRITE`, `STD_FGETS`,
`STD_MALLOC`, `STD_FREE`, `STD_MEMSET`, `STD_MEMCPY`, and `STD_MEMCMP`.

The direct-call inventory requires new `STD_CALLOC`, `STD_FSEEK`, `STD_FTELL`,
`STD_FGETC`, `STD_FPUTC`, `STD_FEOF`, `STD_SNPRINTF`, `STD_TIME`,
`STD_ISSPACE`, and `STD_TOUPPER`. The ctype wrappers cast the input to
`unsigned char`. `fflush(stdin)` remains excluded: it must be replaced by a
platform input-flush capability, not a `STD_FFLUSH` wrapper.

Standard entities receive project spellings without redefining the standard
name: `STD_SIZE_T`, `STD_PTRDIFF_T`, `STD_FILE`, `STD_TIME_T`,
`STD_VA_LIST`, `STD_NULL`, `STD_EOF`, `STD_SEEK_*`, `STD_STDIN`,
`STD_STDOUT`, `STD_STDERR`, and `STD_ATOMIC_*`. `STD_NULL` expands to `NULL`;
the project never redefines `NULL`. Fixed-width `uint*_t`/`int*_t` types stay
unchanged; `UINT8`-style aliases are prohibited because the Windows SDK owns
that namespace.

Win32 adapters currently use `BOOL`, `VOID`, `DWORD`, `WORD`, `BYTE`,
`HANDLE`, `HWND`, `HDC`, `WPARAM`, `LPARAM`, `LRESULT`, `UINT`, `LONG`,
`SHORT`, `LPCSTR`, and direct window/input APIs. Only files below
`platform/win32` may introduce matching `WIN32_*` aliases and API wrappers,
for example `WIN32_BOOL`, `WIN32_DWORD`, `WIN32_HANDLE`, `WIN32_HWND`, and
`WIN32_SLEEP`. `SIZE_T`, `UINT*`, and `BOOL` must not leak into core.

### Adoption Order

1. Define `C_*`, `STD_*`, and `WIN32_*` facade surfaces plus focused smoke
   tests; change no call sites.
2. Rename existing wrappers and direct ISO C calls module by module: core
   machine, core product/platform, VM, VDM, and tests.
3. Migrate C scalar aliases and standard objects/types in the same order.
4. Migrate `WIN32_*` only inside Win32 adapters; use equivalent platform-local
   naming when Linux/macOS adapters exist.
5. Replace debugger input flushing with a platform capability, add a direct
   ISO-header include guard, and run full GCC/FDD/Console gates per task.

This is a new phased adoption plan. It does not reopen M5 or start M6 without
an owner-approved task breakdown.
