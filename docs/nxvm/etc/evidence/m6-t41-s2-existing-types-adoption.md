# M6 T41 S2 — NXVM Existing-Types Adoption Evidence

## Scope and decision

S2 consumes only vocabulary already owned by `src/lib/types`.  It does not
change Lib, delete `type.h` or `type.c`, or infer replacements for a legacy
contract that is not exactly represented by Lib.

The migration touches 469 NXVM production and test C/H files: 21,465 added
lines, 20,996 removed lines, net +469.  The net is one direct
`lib/types/types_interface.h` ownership include per changed caller.  Source
and test code now use the shared scalar, null, boolean, allocation, memory
and selected text vocabulary directly.

## Completed exact mappings

The following legacy names have zero remaining uses in `src/app-nxvm` and
`test/app-nxvm`:

| Legacy facade family | Shared replacement |
| --- | --- |
| `STD_NULL`, `STD_SIZE_T` | `LIB_NULL`, `lib_size` |
| `type_unsigned_{8,16,32,64}` | `lib_u{8,16,32,64}` |
| `type_signed_{8,16,32,64}` | `lib_i{8,16,32,64}` |
| `TYPE_TRUE`, `TYPE_FALSE` | `LIB_TRUE`, `LIB_FALSE` |
| `STD_MEMSET`, `STD_MEMCPY`, `STD_MEMMOVE`, `STD_MEMCMP` | `lib_memory_set`, `lib_memory_copy`, `lib_memory_move`, `lib_memory_compare` |
| `STD_STRLEN`, `STD_STRCMP`, `STD_STRTOK` | `lib_text_length`, `lib_c_strcmp`, `lib_c_strtok` |
| `STD_MALLOC`, `STD_FREE` | `lib_allocate`, `lib_release` |

The one retained `STD_CALLOC` occurrence is deliberate:
`core_machine_ega_registration_transaction_smoke` builds `vadp.c` with
`STD_CALLOC=test_ega_registration_calloc` to prove its allocation-failure
rollback.  Replacing that injected seam with `lib_allocate_zero` bypasses the
test hook.  The normal product still receives the legacy macro's `calloc`
behavior.  This seam moves only when Lib supplies an explicit injectable
allocation contract; it is a S3 design item.

The direct unused `<limits.h>` include in `product/command.c` is removed and
the sole direct `NULL` in `machine/lifecycle.c` now uses `LIB_NULL`.

## Legacy remainder for S3 design

All 439 callers of `type.h` still have at least one retained facade symbol.
They are not overlooked; each belongs to a contract absent from current Lib:

| Remainder | Current count | Why S2 retains it |
| --- | ---: | --- |
| `type_bool` | 739 | Legacy is one byte; `lib_bool` is `int`, so an ABI/layout substitution is invalid. |
| `type_unsigned_4` | 17 | A logical nibble stored in one byte; needs an explicit representation policy. |
| `type_native_unsigned` | 106 | Host-width integer has an architecture contract. |
| `type_virtual_address` | 450 | Integerized host pointer/guest transfer boundary, not a scalar synonym. |
| `type_status` and `TYPE_STATUS_*` | 1,215 | Legacy error/status algebra needs a shared outcome contract. |
| `C_*` primitives, pointer aliases and bit macros | 3,917 `C_VOID` uses plus related forms | Broad C facade and machine-width/bit semantics require a bounded shared design. |
| atomics, formatting, time, ctype, stdio, trace, address and remaining text operations | classified family | Their ownership, ordering, variadic, platform or parsing contracts are not supplied by current Lib. |

`<windows.h>` remains only in explicit Win32 integration/support probes.  It
is a native external boundary, not application production ownership.

## Verification

- Static migration sweep: all completed mapping names above have zero NXVM
  production/test hits; exactly one intentional injected `STD_CALLOC` seam
  remains.
- `vm-0-5-0535` builds as optimized x64 and x86 Release products.
- Repository-only x64 unit suite: **337/337 passed** (parallel 4, 19.58 s).
- Repository-only x86 unit suite: **337/337 passed** (parallel 4, 23.05 s).
- The EGA rollback test passes independently after retaining its injected
  allocator seam: `M5:T395:S1:ROUTE-REGISTRY-SCALABILITY:OK`.
- `library.audio_stream` passes independently and in the successful x64 suite.
- The x86 sweep initially exposed three test-only `0xC00000FD` stack overflows:
  the test programs kept maximum frame fixtures on the default 32-bit stack.
  `vm_machine_frame`, `vm_runner_display_cadence`, and
  `vm_display_composition_s5` now use static-storage fixtures.  They retain
  their assertions and product behavior, pass individually on x86, and the
  complete x86 suite then passes.

## S3 handoff

S3 must propose a compact Lib Types surface and migration order for byte
booleans, host/pointer address types, status outcomes, remaining standard
contracts and C primitive compatibility.  The owner reviews that design before
any removal of `type.h`, `type.c` or the `type-facade` target.
