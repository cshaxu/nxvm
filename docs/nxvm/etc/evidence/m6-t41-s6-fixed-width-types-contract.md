# M6 T41 S6 — Fixed-Width Types Contract

## Result

Shared commit `710cf697b` narrows Lib Types to fixed-width integers,
size/pointer-width values, semantic boolean/status values, and justified C or
SDK boundary vocabulary. It removes `lib_char`, `lib_uchar`, `lib_int`,
`lib_uint`, and `lib_f64`; `lib_void` had already been removed by the prior
corrective S.

The Types layout gate now rejects any future declaration of `lib_void`,
`lib_char`, `lib_uchar`, `lib_int`, `lib_uint`, `lib_f32`, or `lib_f64`. Its
self-test writes a temporary `typedef int lib_int;` declaration and proves the
gate rejects it before restoring the positive control.

## Floating-point rule

Neither `lib_f32` nor `lib_f64` is a spelling for C `float` or `double`.
Either name requires a later explicit IEEE binary format contract and
compile-time representation proof. No current Shared consumer needs one.

## Verification

- Standalone MSVC Debug x64 and x86 Types contract, Types layout and manifest
  tests passed.
- The x64 and x86 Types layout self-tests passed, including the new forbidden
  scalar-alias fixture.
- Lib manifest, direct Types layout verification, documentation governance and
  `git diff --check` passed before commit.

## Product handoff

NXVM, MyNES and SoftPC later migrate internal machine values to fixed-width
or semantic Types. Actual ISO C text, `main`, C runtime and SDK callback
boundaries retain their native C signatures; they are not retyped as machine
integers. SoftPC remains outside this repository and is an explicitly separate
admission.
