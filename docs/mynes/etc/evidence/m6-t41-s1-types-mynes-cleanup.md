# M6 T41 S1 Shared Types and MyNES Cleanup Evidence

## Delivered Surface

Shared commits `343edeb64` and `74a069246` add `lib_i8` and
`lib_c_strstr`, move Base executable-path platform declarations into the
Linux/Win32 Types owners, and refresh both Lib manifests. MyNES uses the
shared vocabulary in production and test code; its native product revision is
`0_0_0041`.

## Static Sweep

The completed sweep ran these queries over `src/app-mynes` and
`test/app-mynes`:

```text
#include <string.h>
memcpy memmove memset memcmp strlen strcmp strcpy strcat strtok strstr
calloc malloc realloc free
int8_t uint8_t int16_t uint16_t int32_t uint32_t int64_t uint64_t
```

Neither query has a remaining hit. The retained test-only boundaries are:

- `battery_persistence_smoke.c` uses stdio to create, corrupt and remove its
  owned temporary storage fixtures.
- `mapper1_smoke.c`, `mapper4_smoke.c`, and `owner_rom_probe.c` read optional
  owner-local environment variables; the probe also emits diagnostics.
- `native_window_smoke.c` and `native_console_smoke.c` call Win32 directly to
  prove the actual presenter boundary.
- `assert.h` remains test-harness vocabulary.

No MyNES production path includes a platform SDK or direct C runtime header.

## Verification

- `library.types_contract`, `library.types-layout`, `library.manifest`,
  `library.types-layout-selftest`, and `library.test-manifest` pass on x64 and
  x86. The layout gate now verifies that Base platform implementations consume
  Types declarations rather than raw Linux/Win32 headers.
- The full MyNES CTest selection `^mynes\.` passes 53/53 on x64 and 53/53 on
  x86, including the native window and console integration checks.
- MyNES x64/x86 Release targets build successfully with post-link stripping.
  The prior MinGW x86 linker `-s` failure is avoided by an equivalent explicit
  `${CMAKE_STRIP} --strip-all` post-link step before artifact copy.

## Artifacts

| Artifact | PE machine | SHA-256 |
| --- | --- | --- |
| `mynes_0_0_0041_x64.exe` | `0x8664` | `DEEB940B400AFE5AD7C575BC775AFD81B6724AD7F645762F139FCBBDC39FEF8D` |
| `mynes_0_0_0041_x86.exe` | `0x014C` | `CC2A53FC2C32AC7F9F8DEC05C3F621AF51616E344F1D01D8FBD762025EDDBE47` |

The sole editable adjacent `assets/binary-mynes/mynes.ini` remains owner-local
and is excluded from this delivery.