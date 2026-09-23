# T512 S3 Five-CPU Current-Code Gap List 2

`M5:T512:S3:FIVE-CPU-LIST-2:ACCEPTED`

## Scope And Disposition Rule

This List 2 consumes every finite F01--F14 expansion in
[List 1](t512-s2-five-cpu-function-state-timing-list-1.md). A row's
`missing` disposition means that current code exists but has not yet received
a complete source-to-form conformance proof; it is not a claim that an opcode
or test is absent. `conflicting` means the original manual and the current
shared mechanism disagree. `conforming` is used only for a directly traced
boundary, not for a whole instruction family merely because a smoke passes.

The rendered 80286 review corrects the earlier source description: Intel
210498-005 Appendix B printed page B-6 defines the `Clocks` field and its
`mem`, `noj`, and `pm` qualifiers. Existing 80286 timing values are therefore
not source-unavailable, but their source-to-selector mapping remains a
`missing` S7 batch until every applicable form and qualifier is traced.

## Shared Current Owners

| Concern | Sole current owner/path | Regression owners | Direct disposition |
| --- | --- | --- | --- |
| Profile admission, opcode space and minimum CPU | `core_machine_cpu_instruction_metadata_get()` in `src/core/machine/cpu_instructions.c`; runtime admission uses `core_machine_cpu_profile_allows_form()` | Five decoder-inventory runners | **Conforming** for the directly tested 8086-only `0F` versus 80286/80386 escaped-space boundary and the listed introduction gates. It is not proof of every instruction semantic. |
| Prefix execution | `_kdf_check_prefix()` and `ExecIns()` in `cpu_instructions.c` | `core_machine_prefix_attributes_s64_smoke.c`; decoder inventories | **Missing** complete legal-form proof. The lexeme scanner deliberately rejects `LOCK` because it cannot decide the target memory-RMW semantic; this preview limitation is not an execution decoder or a second LOCK path. |
| Instruction state, read/write and rollback | Instruction handlers in `cpu_instructions.c`; their transaction/memory helpers | Family smokes listed below | **Missing** per-form source proof; later profile sweeps consume the existing sole owner rather than creating profile implementations. |
| Synchronous failure delivery | `ExecFinal()` in `cpu_instructions.c` | real-exception, protected-mode, VM86, IRET and task-switch smokes | **Missing** complete form/delivery proof; one path owns rollback, vectoring and fault non-retirement. |
| Asynchronous delivery | `ExecInt()` in `cpu_instructions.c` | interrupt-entry and composition smokes | **Missing** complete profile/frame proof; no instruction handler owns a second delivery route. |
| Successful retirement and guest time | `core_machine_cpu_timing_select()` and `core_machine_publish_successful_retirement()` in `src/core/machine/cpu_timing.c` and `machine.c` | five timing manifests, ledger smokes and timing-preview smoke | **Missing** source-to-selector proof per profile. One path selects before the one successful-retirement publication; a delivered fault does not retire. |
| 16-bit FLAGS image/load | `_e_real_flags_defined_mask()`, `_e_real_flags_image_16()`, `_e_real_flags_load_16()` in `cpu_instructions.c` | PUSHF/POPF, IRET, software-INT, ALU, SCAS and TF/DB smokes | **Conflicting** for the 80386 real/V86 bit-15 lifecycle; S4 owns the one shared repair and every affected test oracle sweep. |

## Complete Family-To-Owner Partition

Every profile set below means every decoder-inventory expansion and all
List-1 operand/address/segment/repetition contexts for that family. A row
which names several profile S receivers is partitioned by CPU identity, not by
duplicate code path.

| List-1 family and profile set | Decoder/execution and state owner | Delivery/retirement owner | Current regression owner | Disposition and sole receiver |
| --- | --- | --- | --- | --- |
| F01 accumulator/adjust forms; 8086, 8088, 80186, 80286, 80386DX | primary `insTable` handlers and metadata gate | `ExecFinal()`; sole timing selector/publication | legacy-ALU, five timing manifests | **Missing** complete five-profile source-to-handler/timing proof: S5 (8086/8088), S6 (80186), S7 (80286), S8 (80386DX). |
| F02 arithmetic/logical/compare/test forms; all five | primary `insTable`, ModR/M and memory helpers | same shared delivery/retirement owners | legacy-ALU and per-profile timing suites | **Missing** same per-profile sweep; no form-local fallback is authorized. |
| F03 moves, exchange, segments and 80386 extensions | primary `insTable`, metadata and segment helpers | same shared owners | GPR/segment, protected-data and 80386 paging smokes | **Missing** S5--S8 by admitted profile; F03's 80386-only extensions receive S8, not a VM route. |
| F04 stack, frames, calls/returns and IRET; all applicable profiles | primary `insTable`, stack/segment helpers | `ExecFinal()` and `ExecInt()`; sole selector/publication | PUSHF/POPF, IRET, protected-gate/outer-return, task-switch, VM86 smokes | **Missing**, except the shared FLAGS conflict below: S5--S8 trace each mode/frame and its qualified timing. |
| F05 branches, jumps, calls and loop forms | primary/`0F` tables and control-transfer helpers | same shared owners | control-flow, protected-far/gate and timing smokes | **Missing** S5--S8; 80386 `0F 80-8F`/`SETcc` are S8 only. |
| F06 multiply/divide, bit operations and shifts with arithmetic state | primary/`0F` tables and arithmetic helpers | same shared owners | legacy-ALU, timing manifests, 80386 timing smoke | **Missing** S5--S8; dynamic/range selection must be reconciled at the one timing selector. |
| F07 group-2 shifts/rotates; all legal count forms | primary `insTable`, ModR/M helpers | same shared owners | group/legacy-ALU and timing smokes | **Missing** S5--S8; 80186 immediate counts begin in S6 and 80386 widths in S8. |
| F08 strings and 80186 `INS/OUTS` | string/I/O handlers and repeat state | same shared owners | string, port-string and timing smokes | **Missing** S5--S8; each actual repeat/termination outcome must map to one existing retirement result. |
| F09 port, wait, escape, halt and miscellaneous forms | primary table; FPU escape dispatch remains Core-local | `ExecFinal()` plus sole retirement path | port, FPU-interface/escape and timing smokes | **Missing** S5--S8. F14 is separately retained for the CPU/FPU boundary. |
| F10 FLAGS controls and image/load; all five | shared FLAGS helpers and flag-control handlers | `ExecFinal()`/`ExecInt()` frame image paths | direct-FLAGS, PUSHF/POPF, IRET, software-INT, TF/DB, SCAS, legacy-ALU | **Conflicting**: Intel 80386DX states that real/V86 stack images make bit 15 clear, while the current 80386 16-bit mask is `FFD5h`; two test helpers even use `FFFFh`. S4 must correct the one Core canonicalization and sweep all image/load/frame callers. |
| F11 80186 additions | metadata minimum-CPU gate and primary handlers | shared owners | 80186 decoder/timing runners | **Conforming** absence gate below 80186; **missing** 80186 source-to-handler/timing proof, receiver S6. |
| F12 80286 protected forms | `0F` metadata/table and protected-mode helpers | `ExecFinal()` protected delivery | 80286 protected, protected-gate/return/task and timing smokes | **Conforming** absence gate below 80286; **missing** complete protected-form, delivery and Appendix-B timing-qualifier proof, receiver S7. |
| F13 80386DX system, paging, VM86 and 32-bit forms | `0F` metadata/table and 80386 helpers | `ExecFinal()`/`ExecInt()` and shared retirement | 80386 decoder/paging/protected-I/O, VM86 and timing smokes | **Conforming** absence gate below 80386; **missing** complete 80386DX source-to-handler/delivery/timing proof, receiver S8. |
| F14 x87 escape bytes; all applicable CPU profiles | primary escape admission plus `core_machine_fpu_escape_dispatch()` | CPU `#NM`/unsupported delivery and CPU retirement; FPU operation remains its own owner | FPU 8087/interface/profile-closure and escape smokes | **Missing** complete CPU/FPU boundary audit, receiver S4 for shared escape/frame state and S5--S8 for profile-specific timing/availability. No CPU `#UD` compatibility branch is permitted. |

## FLAGS Conflict: Exact Bounded Batch

Intel's 80386DX manual identifies the affected **image** contexts: `PUSHF`,
interrupts, and exceptions in real-address mode; its virtual-8086 section
states the same bit-15 rule. Current code applies one `FFD5h` mask to both
image and 16-bit load canonicalization. The implementation is deliberately
single-owner, but the owner selects the wrong 80386 defined-field set for the
required image contract. S4 must first distinguish source-defined image,
defined load fields and intentionally canonicalized undefined state, then
update the one helper and every named test oracle together. It must not assert
an exact value for any remaining undefined field.

## Timing And Prefix Partition

| Batch | Current owner | Source/status | Receiver |
| --- | --- | --- | --- |
| 8086 base plus 8088 transfer adjustment | primary/string/control-stack selectors and 8086/8088 manifests | Manual tables exist; exact selector/formula coverage is **missing** | S5 |
| 80186 forms/ranges | 80186 selector chain and manifest/ledger smokes | Manual table exists; exact row/range coverage is **missing** | S6 |
| 80286 Appendix-B clocks | 80286 selector chain and `core_machine_80286_source_timing_ledger` | Manual-L3 candidate, including `mem`/`noj`/`pm`; exact selector/qualifier coverage is **missing**, not source-unavailable | S7 |
| 80386DX forms, modes and ranges | 80386 selector chain and manifests | Manual table exists; exact selector/formula/mode coverage is **missing** | S8 |
| `LOCK`, segment, repeat and 80386 size/FS/GS prefixes | runtime prefix parser plus metadata; lexeme preview intentionally omits target-dependent `LOCK` | Prefix-by-prefix legal-target proof is **missing** | S5--S8 by profile |

## Repair Partition And No-Parallel-Path Rule

S4 is the only shared-mechanism receiver: it owns the FLAGS-image/load/frame
conflict and any equivalent helper discovered by its bounded caller sweep.
S5--S8 are mutually exclusive profile batches and must use the current Core
decoder, execution, delivery and retirement owners. No S may move CPU
behavior into a VM profile, retain an old timing selector beside a new one, or
make a test oracle define a reserved bit value.

`M5:T512:S3:OWNER-PARTITION:READY`

## Executor Verification

- Documentation governance passed after the source-locator correction and List
  2 addition.
- Complete repository-only unit suite: 313/313 passed with
  `ctest --test-dir build\\mingw-gcc-x64 -L unit -j 8 --output-on-failure`
  (27.07 seconds real time).

## Coordinator Review

The actual S3 diff is evidence-only: it adds the finite owner partition,
corrects the 80286 original-manual locator, and updates the active record. It
does not alter a CPU decoder, test oracle, VM profile, build output, or guest
asset. The rendered source rule, direct shared-mask trace and full unit result
accept the List 2 partition. S4 is the next eligible receiver.
