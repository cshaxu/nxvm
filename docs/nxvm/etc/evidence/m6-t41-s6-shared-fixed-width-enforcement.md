# M6 T41 S6 — Shared Fixed-Width Enforcement Evidence

## Scope

S6 consumes the six shared corpora: `src/lib`, `src/common`, `src/x86`,
`test/lib`, `test/common`, and `test/x86`.  It does not migrate either product
tree or retire NXVM's legacy `type.h`/`type.c` facade.

## Disposition

- `lib_bool` and `lib_status` now explicitly use `lib_i32`; queue, input,
  presentation and xasm32 state use fixed-width or semantic Types rather than
  bare scalar spellings.
- Window geometry, mouse motion, session queue capacity, frame state, test
  counters and x86 assembler mode now use `lib_i32`, `lib_u32`, `lib_size` or
  `lib_bool` according to their value contract.
- The removed Types surface has no shared production consumer: ctype wrappers,
  stream-format aliases, obsolete atomics, unused platform aliases and
  `LIB_UINT_MAX` are gone.  `lib_c_strstr` remains only as the named MyNES S7
  C-string test adapter.
- Atomic `u64` now names exact `lib_u64` storage on C11 hosts.  C/SDK scalar
  spellings remain only at their immediate runtime/platform bridge, and test
  `int main` plus explicit Linux ABI fixtures remain direct probes.

## Verification

- All six manifests and Lib/Common/x86 static boundary checks pass.
- Shared Common CTest: x64 **14/14**, x86 **14/14**.
- Shared x86 CTest: x64 **6/6**, x86 **6/6**.
- Shared Lib CTest excluding the pre-existing
  `library.kvm_window_capture_contract` crash: x64 **36/36**, x86 **36/36**.
  The excluded target still crashes in the unchanged baseline and is not
  attributed to this S.

## Handoff

S7 removes the retained `lib_c_strstr` MyNES test adapter while migrating
MyNES-owned text and scalar use.  S8 performs the NXVM facade migration and
deletes `type.h`/`type.c` only after that product's live receivers move.
