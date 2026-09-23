# M6 T41 S4 — Shared Lib Types Foundation

## Scope

Shared commit `9d7f6ba6c` establishes the neutral Lib Types foundation needed
for the later NXVM root-facade migration. It changes only `src/lib` and
`test/lib`; no NXVM or MyNES product source, artifact, asset, or local INI is
part of the commit.

## Delivered contract

- `types_interface.h` supplies the required C scalar aliases, `lib_uptr`,
  checked object-pointer conversion helpers, and the generic status taxonomy.
  `LIB_STATUS_INTERNAL_ERROR` is ordinal 6; no existing Lib status moved.
- `atomic.h` supplies pointer-width initialize, explicit load, and strong CAS
  operations. The MSVC implementation selects by pointer width rather than by
  operating system; C11 uses `atomic_uintptr_t`.
- `file.h` supplies direct ISO C `stdout`, `stderr`, `fprintf`, and
  `vfprintf` vocabulary. Character classification wrappers first convert
  through `unsigned char`.
- `LIB_STATUS_NOT_CURRENT` is deleted. Stale Console delivery and detached
  output are successful no-ops, cooked-line mode/current misuse is
  `INVALID_STATE`, and the Linux presenter stub returns `UNSUPPORTED`.
- The KVM Console worker acknowledges a published frame only after a genuine
  successful output write.

`lib_f32` was deliberately not added: the S3 proposal made it conditional on
an actual remaining facade contract, and the NXVM source/test sweep has no
`C_FLOAT` or `type_float` consumer.

## Verification

- Fresh standalone MSVC Debug configure/build completed for x64 and x86.
- On each architecture, all 44 non-desktop `test/lib` cases passed, including
  Types, Console stale-delivery, Console Broker, and KVM Console retirement
  coverage.
- Lib and Lib-test manifests, Types layout, component dependency, and
  documentation governance checks passed. `git diff --check` passed before
  commit.
- `kvm_window_capture_contract` separately terminates with an access violation
  on both x64 and x86, with no output. It is unchanged by this Shared commit
  and links only unchanged KVM Window code, so it is recorded as a pre-existing
  unrelated test defect rather than hidden by this packet. It remains a
  separate receiver before any full-suite green claim.

## Handoff

S5 migrates NXVM data representation callers to this contract: scalar and
pointer aliases, byte-layout booleans, named status outcomes, and the three
logical flags using existing `lib_atomic_i32`. It must prove every changed
layout and status mapping on x64 and x86 before moving to bit operations.

## Corrective addendum

After review, the owner identified `lib_void` as an unjustified duplicate of
the C keyword. The corrective S removes it; NXVM `C_VOID` callers will migrate
directly to `void`. The scalar-alias sweep distinguishes this from width and
semantic contracts: `lib_u8/u16/u32/u64`, `lib_size`, `lib_iptr`, `lib_uptr`,
`lib_bool`, and `lib_status` retain a width or behavior purpose. The remaining
plain C scalar aliases are not adopted by any Lib implementation and require a
separate migration-design decision before NXVM uses them.
