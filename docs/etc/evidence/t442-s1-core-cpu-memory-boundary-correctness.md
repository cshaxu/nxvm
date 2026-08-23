# M5 T442 S1 Core CPU And Physical-Memory Boundary Correctness

## Scope And Result

T442 repairs two independent Core-private input boundaries without adding a
layer, ABI, caller-side validation path, or emulation feature.

- `core_machine_cpu_instruction_lexeme_scan_with_options` now expresses the
  complete `0F` profile partition directly: 8086 retains the primary opcode,
  80186 rejects it, and 80286/80386 consume the extended opcode.
- `core_machine_memory_register_mapping` rejects a physical `[start,
  start + bytes)` span beyond the 32-bit address space before it increments
  `mapping_count` or writes a mapping.

The first attempted CPU simplification used `profile >= 80286` as the outer
condition. Its focused regression showed that 80186 would then incorrectly
fall through to the 8086 primary-opcode path. The final two-comparison finite
predicate is therefore necessary behavior, not defensive layering.

## Focused Proof

`core-machine-cpu-timing-preview-smoke` passed and emitted:

```
M5:T442:S1:CPU-LEXEME-PROFILE-BOUNDARY:OK
```

The system-group test now proves the 8086 one-byte `0F` lexeme, retains the
existing 80186 rejection, and retains the existing legal 80286/80386 extended
forms.

`core-machine-ram-port-context-smoke` passed and emitted:

```
M5:T442:S1:PHYSICAL-MAPPING-SPAN-BOUNDARY:OK
```

It accepts `start=fffffff0h, bytes=16` and rejects `bytes=17`; the rejection
also proves that `mapping_count` remains unchanged.

## Similar-Issue Sweep

| Path or shape | Disposition |
| --- | --- |
| CPU lexeme `0F` guard | Repaired at its one lexical owner. The executor's 80286+ `INS_0F` dispatch and later opcode metadata are execution/decode consumers, not a second lexical-prefix decision. |
| RAM device-provider registration | Already computes a 64-bit endpoint before publication; separate provider-capacity and overlap semantics retained. |
| Immutable ROM mapping registration | Already rejects a 64-bit physical endpoint before registering its immutable mapping; separate owner retained. |
| Absent-memory configuration | Already rejects a 64-bit physical endpoint before configuration publication; separate machine configuration owner retained. |
| RAM/device/ROM access resolution | Each first checks the physical start and then uses a 64-bit offset; no registration mutation occurs and no duplicate endpoint guard is added. |

No helper was introduced: the physical-span calculation occurs once at the
mapping registration owner, while the other listed calculations own different
objects and failure contracts.

## Change Accounting And Delivery

The implementation changes production CPU/memory code by `+6/-3` lines and
the two directly owning smoke tests by `+12/-1` lines; CMake advances the
current developer artifact from 0441 to 0442. The former contradictory
80186 admission branch is removed. No parallel validator, state copy, or
compatibility path remains.

Focused commands:

```
cmake --build build\\mingw-gcc-x64 --target core-machine-cpu-timing-preview-smoke core-machine-ram-port-context-smoke
ctest --test-dir build\\mingw-gcc-x64 -V -R current.core-machine-cpu-timing-preview-smoke
ctest --test-dir build\\mingw-gcc-x64 -V -R current.core-machine-ram-port-context-smoke
```

The new developer artifact is `vm-0-5-0442` at
`build/output/nxvm_0_5_0442.exe`, SHA-256
`B94C6355E4ADD9EF982E4713A15860F23D4B24BC912BBA9608D4DA3B7A2BBE1E`.

Documentation governance and the current-fast build both passed. The complete
current gate reached its existing independent T344 historical-fixture failure:
it expects 71 direct machine constructors and finds 75. The artifact-truth,
documentation, and every subsequently scheduled verification through that
point passed, including the 0442 artifact identity check. T442 changes neither
the T344 verifier nor machine-constructor fixtures.
