# M6 T41 S3 — Lib Types Facade-Retirement Design

## Decision

Retire NXVM's root `type.h`, `type.c`, and `type-facade` target by moving only
neutral, cross-product C representation contracts into Lib Types.  NXVM then
uses ordinary C only where it is clearer than a project alias, and keeps
machine policy, console policy, x86 trace control, and test fault injection at
their actual owners.

This is a design record.  It authorizes no source migration or deletion until
the owner reviews it.

## Evidence baseline

S2 exhausted every exact existing Lib equivalent.  The remaining source and
test corpus contains 439 root-header callers and these material families:

| Family | Evidence | Design receiver |
| --- | ---: | --- |
| `C_INT`, `C_VOID`, `C_CHAR`, `C_UCHAR`, `C_UINT` | 9,366 direct uses | New minimal Lib scalar spelling, or direct ISO C where no public alias is needed. |
| `type_bool` | 770 uses | `lib_u8`; it preserves the one-byte field and pointer ABI. `lib_bool` remains only for logical local results. |
| host/pointer widths | 568 uses of `type_virtual_address`/`type_native_unsigned` plus one pointer alias | New `lib_uptr` and explicit pointer conversion helpers. |
| outcome algebra | 1,221 `type_status`, 6,986 status constants | Remove Console-only `NOT_CURRENT`; add the general `LIB_STATUS_INTERNAL_ERROR`; migrate source semantics after proving status values are not serialized or numerically ordered. |
| atomic vocabulary | one pointer-width nonce and three stop/reset flags | Add the missing neutral pointer-width atomic; represent NXVM's flags with existing `lib_atomic_i32` and explicit ordering. |
| fixed sub-byte/nonstandard widths | 17 `type_unsigned_4`, plus masks | Use storage-width `lib_u8/u32/u64` and explicit typed C masks, never fictional C bit-width types. |
| bit, BCD and address macros | 2,000+ material uses | Explicit typed C expressions at their owners; x86/RTC-specific BCD helpers stay with their device owner. |
| trace macros | 5,200+ instruction-decoder uses | New local `cpu_trace` compatibility header, then direct decoder refactor; never Lib. |
| C stream/text forwarding | 1,415 `STD_PRINTF`, 182 `STD_FPRINTF`, and sparse helpers | Existing `lib/types/file.h` for ISO stream vocabulary; product/test output adapters for output policy. |
| allocator injection | one live `STD_CALLOC` seam | NXVM test-only allocator dependency; ordinary code uses `lib_allocate_zero`. |

The count is intentionally descriptive rather than a deletion gate.  Later
implementation must produce a zero-reference scan for each retired family.

## Shared Lib Types additions

### Scalar and address contract

`types_interface.h` gains only the aliases that replace an actual public
facade contract:

```c
typedef void lib_void;
typedef char lib_char;
typedef unsigned char lib_uchar;
typedef int lib_int;
typedef unsigned int lib_uint;
typedef float lib_f32;
typedef double lib_f64;
typedef uintptr_t lib_uptr;

_Static_assert(sizeof(lib_uptr) == sizeof(void *),
    "lib_uptr must preserve a host pointer");

#define LIB_UPTR_MAX UINTPTR_MAX

static inline lib_uptr lib_pointer_to_uptr(const void *pointer);
static inline void *lib_uptr_to_pointer(lib_uptr value);
```

`lib_uptr` replaces the legacy native unsigned, virtual address and unsigned
pointer aliases.  `lib_iptr` already exists and replaces the signed form.
Callers whose value is a guest physical or linear address remain `lib_u32`;
the migration must not widen guest architectural addresses merely because a
host pointer is present elsewhere.

`type_bool` becomes `lib_u8` at stored ABI boundaries.  `lib_bool` stays an
`int` and is reserved for predicates, conditions and atomic API returns.
This distinction is tested with struct-size and field-offset fixtures on x64
and x86.

### Outcome contract

`lib_status` is the authoritative product-neutral outcome type.  Its final
shared vocabulary is:

```c
LIB_STATUS_OK = 0
LIB_STATUS_INVALID_ARGUMENT = 1
LIB_STATUS_INVALID_STATE = 2
LIB_STATUS_UNSUPPORTED = 3
LIB_STATUS_NO_MEMORY = 4
LIB_STATUS_IO_ERROR = 5
LIB_STATUS_INTERNAL_ERROR = 6
LIB_STATUS_LIMIT_EXCEEDED = 7
```

`LIB_STATUS_NOT_CURRENT` is deleted.  It is a Console/KVM generation detail,
not a product-neutral failure: stale asynchronous frames become successful
no-ops, wrong Console operation mode becomes `INVALID_STATE`, and absent
platform presenters return `UNSUPPORTED`.

The root `TYPE_STATUS_FAULT` currently has ordinal 5, while shared
`LIB_STATUS_IO_ERROR` already owns ordinal 5.  The migration therefore cannot
preserve the old numeric value without conflating distinct meanings.  It
maps only genuine completed-operation internal failures to
`LIB_STATUS_INTERNAL_ERROR = 6`.  The primary NXVM example is a machine that
has entered `CORE_MACHINE_STOP_FAULT`, including execution-invariant or timing
overflow failure, for which the run result already carries a machine-specific
detail code.  Existing Lib values do not move.

The remaining root fault uses are not grandfathered as a catchall: mapping
callbacks with invalid input become `INVALID_ARGUMENT` or `UNSUPPORTED`, DMA
nonce exhaustion becomes `LIMIT_EXCEEDED`, resource-content mismatch and
host loading failure become `IO_ERROR`, and composition paths preserve their
specific status.  This keeps the shared status vocabulary general and makes
NXVM's detailed machine fault a core concern rather than a Lib subtype.

Before that change, P2 audits that NXVM neither serializes status numbers nor
uses numeric ordering/ranges.  The known non-comparison uses place a raw status
value into in-process memory/port trace details and print it in test failure
messages; P2 converts their interpretation and assertions to named outcome
semantics.  They are not a persistence or external ABI.  `lib_status` remains
an `int`; dual-architecture fixtures assert its representation remains
compatible with the retired enum at every exposed function boundary.

### Atomic contract

The existing Lib atomics cover flag, signed 32-bit, unsigned 32-bit and
unsigned 64-bit storage.  They do not cover the two root uses precisely:

* DMA allocates opaque binding nonces with one `atomic_uintptr_t` load and
  strong compare-exchange.  P1 adds `lib_atomic_uptr`, with initialize, load
  and strong compare-exchange explicit operations.  Its storage has pointer
  width on both x86 and x64; tests cover the returned observed value on a CAS
  failure and the `LIB_UPTR_MAX` exhaustion sentinel.
* Machine stop/reset fields are logical flags.  They migrate to existing
  `lib_atomic_i32`, storing `LIB_FALSE`/`LIB_TRUE`, rather than adding a
  misleading byte-boolean atomic facade.  P2 audits their containing structs
  as internal-only and records the x86/x64 layout change before accepting it.

Every root non-explicit atomic operation maps to its explicit Lib equivalent
with `LIB_MEMORY_ORDER_SEQ_CST`, preserving C11's default ordering.  The Lib
implementation of `lib_atomic_uptr` uses C11 `atomic_uintptr_t` where present
and the platform-width native interlocked primitive on MSVC; both variants are
validated on x64 and x86.  It exposes no NXVM policy or machine state.

### Bit contract

Do not add `lib_bits`.  The legacy bit/mask macros are mechanical expressions,
not a reusable behavioral contract.  They migrate to direct typed C
expressions and explicit `lib_u8/u16/u32/u64` casts at their register owner.
This prevents hidden multiple evaluation, preserves visible register-width
truncation, and avoids a redundant utility surface.  The `TYPE_GET/SET/CLEAR/
MAKE_BIT`, mask and MSB/LSB macros migrate in compiler-checked batches with
focused decoder, DMA, PIC, PIT, FDC and RTC tests.

`TYPE_HEX_TO_BCD` and `TYPE_BCD_TO_HEX` are not Lib Types additions.  They
belong to the RTC/device owner because their valid range and invalid BCD policy
are machine behavior.  Likewise, `TYPE_EXECUTE_FUNCTION` is deleted or
replaced locally only after confirming every call site is dead.

### ISO C stream vocabulary

Expand `lib/types/file.h` only with direct ISO C vocabulary needed by shared
callers:

```c
#define lib_c_stdout stdout
#define lib_c_stderr stderr
#define lib_c_fprintf fprintf
#define lib_c_printf printf
#define lib_c_fflush fflush
```

`STD_SNPRINTF`, `STD_VSNPRINTF`, `STD_VA_LIST` and stream type already have
Lib receivers.  Character classification and case conversion must be safe
`static inline` wrappers that cast their input through `unsigned char`, as the
root facade does; direct macro aliases to `isalpha` or `isspace` would be
incorrect for negative `char` values.

The root `STD_TIME`, `STD_LOCALTIME`, `STD_STRCAT`, `STD_STRCPY`, `STD_STRTOK`
and `STD_ATOI` wrappers have no executable NXVM caller; the apparent
`STD_TIME` hits are FDC comments.  They are deleted with `type.c`, not moved.
`STD_SNPRINTF_APPEND` has one root-only smoke test and no NXVM product caller.
That smoke target is retired with the wrapper.  The separately retained x86
DEBUG command implementation has a different local truncation-result contract,
so it is not a valid reason to make either behavior a shared Lib API.

`STD_PRINTF` currently flushes after each call.  That is product/test output
policy, not a Types operation.  Production callers move to their app output
adapter; test probes use a test output helper.  `STD_FPRINTF` retains ordinary
stream semantics through `lib_c_fprintf`.

`STD_TIME`/`STD_LOCALTIME` must not be exposed from Types: elapsed-time callers
use Base clock, while wall-clock calendar conversion needs a separately
designed Base time API if it remains live.  `type_string_lower` becomes the
owner-local parser helper; it is not a generic Types contract.

## Explicit allocator-test redesign

The sole remaining `STD_CALLOC` exists because
`core_machine_ega_registration_transaction_smoke` compiles `vadp.c` with a
macro override.  The final migration replaces this preprocessor interception
with a `core_machine_vadp_test_allocator` passed only through the test setup or
an internal allocation callback on the EGA configuration path.  Normal
production construction calls `lib_allocate_zero`; the test callback can fail
one requested allocation and observes call count.  The callback is NXVM test
infrastructure, not a Lib public allocator API.

## Migration sequence

1. **Shared P1 — scalar/status/atomic contract.** Add the proposed Lib Types
   surface, remove `NOT_CURRENT`, and add dual-architecture layout, status and
   atomic tests.  Audit status numeric escape paths before adding
   `LIB_STATUS_INTERNAL_ERROR`; update Lib manifests.
2. **NXVM P2 — data representation.** Migrate primitive aliases, byte booleans,
   pointer-width values, `type_status` and the three internal atomic flags;
   replace status-number diagnostic expectations with named outcomes; assert
   every applicable public layout and x86/x64 product build.
3. **NXVM P3 — pure operations.** Replace bit/mask/reference/dereference
   macros by direct typed expressions or explicit casts.  Audit each x86
   device family and its focused unit corpus.
4. **NXVM P4 — runtime and local policy.** Move stream naming to `file.h`,
   output to product/test adapters, time/text/BCD to their owners, and install
   explicit EGA test allocation injection.
5. **NXVM P5 — decoder trace retirement.** Replace no-op root trace macros in
   `cpu_instructions` with a local decoder-only header or remove them in a
   compilation-preserving change.  This is intentionally isolated from shared
   Lib.
6. **NXVM P6 — delete and prove.** Remove `src/type.h`, `src/type.c`, the
   `type-facade` target and every link edge; remove its entries from NXVM's
   strict-production target lists and target-classification rules; retire the
   root-only `std-snprintf-smoke`; reject root-header includes and legacy
   facade tokens in NXVM corpus checks.  Rebuild all products and run both
   architecture suites.

## Acceptance and deletion proof

Before P6 closes:

- `rg` finds no `#include "type.h"`, `type-facade`, `src/type.c`, `C_*`,
  `STD_*`, `TYPE_*` or `type_*` legacy facade use in the NXVM source/test
  corpus, except explicitly retained historical documentation;
- all new Lib public declarations have ownership/layout/behavior tests and
  manifests on x64 and x86;
- current NXVM repository unit suite passes on x64 and x86, and every product
  target rebuilds as optimized Release;
- no Lib addition refers to NXVM, x86 machine state, console policy or test
  harness behavior; and
- SoftPC/MyNES adoption remains optional and source-compatible: they consume
  the new generic Lib surface only when separately admitted.

The build-graph audit proves that neither MyNES nor Lib includes root `type.h`;
the facade is an NXVM-only target.  Its removal must nevertheless update every
NXVM governance list that currently names it, so the configuration remains
self-consistent rather than merely compiling by accident.

## Review decisions requested

1. Approve `lib_uptr` and `lib_u8` as the explicit address/byte-boolean
   replacements rather than reshaping `lib_bool`.
2. Approve removal of `NOT_CURRENT` and `LIB_STATUS_INTERNAL_ERROR = 6` as a
   source-semantic outcome: the root fault ordinal changes because shared
   `IO_ERROR` already owns ordinal 5.
3. Confirm no `lib_bits` surface: direct typed expressions own bit behavior;
   BCD, trace and output policy remain outside Lib Types.
4. Approve explicit NXVM test allocation injection in place of the global
   `STD_CALLOC` compile override.
