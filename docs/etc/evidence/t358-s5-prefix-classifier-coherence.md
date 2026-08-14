# M5 T358 S5: Prefix Classifier Coherence

## Scope And Audit Method

This audit follows each Intel 80386 instruction-prefix decision from byte
classification through the handler's state publication. It covers segment
overrides, `F2`/`F3`, `66`, `67`, and `F0 LOCK`; it does not claim later-CPU
prefix policy, prefetch timing, or opcode-family completeness.

The source sweep used:

```text
rg -n "_kdf_check_prefix|lexeme_is_prefix|prefix_(rep|oprsize|addrsize)|PREFIX_(LOCK|REP|ES|CS|SS|DS|FS|GS)|flagLock|roverds|roverss" src/core/machine/cpu_instructions.c
rg -n "prefix_rep" src/core/machine/cpu_instructions.c
rg -n "PREFIX|LOCK|66|67|F2|F3" tests/machine/core_machine_prefix_attributes_s64_smoke.c tests/machine/core_machine_cpu_timing_preview_smoke.c
```

## Decision-To-Publication Ledger

| Mechanism | Single owner and consumers | Publication/fault disposition |
| --- | --- | --- |
| Prefix byte acceptance | `_kdf_check_prefix` is the runtime recognizer used by `ExecIns` and the `LOCK` look-ahead; `core_machine_cpu_instruction_lexeme_is_prefix` is the non-mutating timing-preview counterpart. `64/65/66/67` require 80386; the original segment and repeat bytes are available on every selected profile. | Pre-386 `64/65/66/67` reaches `#UD` through normal dispatch. Preview has no execution side effect. |
| Segment overrides | `PREFIX_ES/CS/SS/DS/FS/GS` set the one `roverds`/`roverss` selector used by `_kdf_modrm` and source-string helpers. The last segment prefix wins. | Destination-fixed ES string operations retain their own destination route; no override creates a new state mirror. |
| Operand and address size | `PREFIX_OprSize` and `PREFIX_AddrSize` set presence booleans; `_GetOperandSize` and `_GetAddressSize` apply the one 16/32 default inversion. Runtime repeated prefixes therefore remain an idempotent presence attribute. | The timing-preview scanner had incorrectly toggled each repeated `66`/`67`, disagreeing with runtime and producing a wrong length/EA classification. It now records prefix presence before calculating the one inversion. |
| Repeat | `PREFIX_REPNZ`/`PREFIX_REPZ` set one last-prefix-wins enum. `INS/OUTS`, `MOVS`, `STOS`, `LODS`, `CMPS`, and `SCAS` consume it; CMPS/SCAS additionally stop according to ZF. | Each repeated execution retires one primitive and leaves the restart/IRQ boundary to the established string path. S64 owner proof covers zero/one/multiple, F2/F3 and mixed-prefix last-wins behavior. |
| 80386 LOCK | `PREFIX_LOCK` is the sole early validity classifier. `_d_modrm` rejects an otherwise valid locked register destination before operand access. | The classifier formerly admitted several read-only or invalid group forms, allowing memory forms to execute or fetch before `#UD`. It now admits only memory-capable modifying forms: arithmetic groups `/0`--`/6`, `F6/F7 /2,/3`, `FE/FF /0,/1`, and `0F` BTS/BTR/BTC forms (including `0F BA /5`--`/7`). |
| Preview lock | `core_machine_cpu_instruction_lexeme_scan` intentionally reports a LOCK-prefixed instruction unavailable: it is a timing-preview capability, not runtime decode and has no owner-side execution path. | Locked timing rows remain an explicit transfer to the complete instruction-timing corpus; this is not a `#UD` classification bypass. |

## Reproduced Repairs

1. `F0 0F A3`, `F0 0F BA /4`, `F0 F6/F7 /0`, `F0 FE /2`, and `F0 FF /2`
   were accepted by the old broad `PREFIX_LOCK` whitelist. Memory forms could
   reach their handlers, contrary to the 80386 LOCK-validity rule. The shared
   classifier now rejects those group variants before operand decoding. Owner
   regressions prove `#UD`, old CPU state, and candidate memory
   nonpublication; legal locked NOT, NEG, and INC memory forms still publish
   their expected write.
2. The timing-preview scanner toggled every repeated `66`/`67`, unlike the
   runtime presence booleans. `66 66 B8 id` and `67 67 8A 06` now prove the
   scanner and executor agree on 32-bit operand and 32-bit addressing,
   respectively, including exact byte count and selected memory candidate.

## Retained Proof And Transfers

`core-machine-prefix-attributes-s64-smoke` proves six segment overrides,
segment and repeat last-wins, fixed-ES strings, independent and combined
width attributes, legacy rejection, legal and illegal LOCK, repeat edges, and
the pending-IRQ boundary. `core-machine-cpu-timing-preview-smoke` proves the
nonmutating layout result. Existing MOVS/STOS/LODS/SCAS/CMPS and port-string
owners retain their detailed protected, VM86, fault, and restart matrices.

The following are deliberately outside S5: exhaustive opcode timing for LOCK
forms; later-CPU/VME prefix policy; prefetch/physical-lock bus semantics; and
unadmitted opcode-family behavior. They remain transfers to the ordered
instruction-timing, bus-timed, and cycle-exact Queue candidates.

## Verification

- Focused `core-machine-prefix-attributes-s64-smoke` passed with
  `M5:T316:S64:PREFIX-ATTRIBUTES:OK` after the new illegal-group and legal-write
  vectors.
- Focused `core-machine-cpu-timing-preview-smoke` passed with
  `M5:T357:S2:CPU-TIMING-PREVIEW:OK` after the repeated-width vectors.
- Fresh `mingw-gcc-x64` configuration and parallel current gate passed
  240/240 tests.
- `verify-current-artifact-target` passed and the rebuilt
  `nxvm_0_5_0358.exe` SHA-256 remains
  `59FAC0E73FED23BA47F3F25C1946C8D2949CFB186C480CE9841588E47E754E2E`.
