# T390 S4: Model-40 Preboot Retirement Matrix

`M5:T390:S4:PREBOOT-RETIREMENT-MATRIX:OK`

## Scope And Containment

S4 uses the accepted copied Core retirement observer with an owner-managed,
uncommitted Model-40 firmware pair. The local invocation receives the existing
BYOB manifest values from the owner-managed environment; no firmware/media
path, filename, digest, provenance text, byte content, guest trace or local
output is retained here. Its executable is an explicit operator-run research
tool, not a default runtime dependency or a current-gate test.

The run resets one selected Model-40 session and observes the first 64
successful firmware retirements, one instruction per call. It terminates
normally at that finite preboot checkpoint. Temporary normalized output is
deleted after process exit. The observed matrix has 64 records, 17 distinct
normalized form/tick rows, 64 classified records and zero source-unallocated
records. Every row is real-address, CPL 0, 16-bit operand/address default, no
LOCK and no repeat prefix.

## Exact-Row Crosswalk

The Intel 80386 Programmer's Reference Manual Table 8-1 is the primary source
for selected real-address rows and the `m` lexical additions used by the
existing direct-control source owner. Rows are CPU clocks under the manual's
prefetched/no-wait/no-HOLD assumptions, not DeskPro bus, ROM, device or board
time. The original Intel manual is retained by
[Bitsavers](https://www.bitsavers.org/components/intel/80386/230985-001_80386_Programmers_Reference_Manual_1986.pdf);
the Table-8-1 crosscheck used [the historical Intel manual scan](https://audio.manualsonline.com/manuals/mfg/intel/80386.html?p=173).

| Normalized observed form | Context/result | Ticks | Exact-row disposition |
| --- | --- | ---: | --- |
| `lmsw-register` | real, register operand | 10 | Existing privileged 80386 owner; Table 8-1 register row. |
| `xor-register-register` | real, register operands | 2 | Existing primary ALU owner. |
| `cmp-register-immediate` | real, register operand | 2 | Existing primary immediate-ALU owner. |
| `test-accumulator-immediate` | real accumulator immediate | 2 | Existing primary TEST owner. |
| `mov-immediate`; `mov-register-register` | real register forms | 2 | Existing 80386 ledger rows. |
| `mov-sreg` | real, register source | 2 | S4 exact `MOV Sreg,r/m16` register row; memory remains unallocated. |
| `sahf`; `cli` | real; CLI at CPL 0 | 3 | S4 exact rows. |
| `lods` | real, non-repeat | 5 | Existing string source owner. |
| `in-immediate`; `out-immediate`; `out-dx` | real I/O forms | 12; 10; 11 | Existing 80386 I/O ledger rows. |
| `jcc` | taken target, two lexical components | 9 | Existing `7 + m` owner; lexical receiver retained by T388 S5. |
| `loop` | observed taken form | 12 | Existing control-transfer source owner. |
| `jmp-direct`; `jmp-far-direct` | direct target, two lexical components | 9; 14 | Existing direct-control source owner. |

Only observed no-prefix, real-address `CLI`, `SAHF`, and register-source
`MOV Sreg,r/m16` enter the new 80386 ledger entries. Prefixes remain
source-unallocated. A memory-source `MOV Sreg,r/m16` remains source-unallocated
and the physical gate rejects it before elapsed-time publication.

## Source And Caller Sweep

The classifier order remains one owner: string/I-O, dynamic, secondary,
privileged, primary/control-stack, then the profile-specific 80386 fallback.
The S4 forms are not handled by an earlier matching 80386 owner. The new
fallback cases share the existing ledger lookup; no profile execution route,
second time publisher or public API is added. The physical-contract check
remains after observation and before `core_machine_publish_elapsed_ticks()`.

The static sweep covers CLI, SAHF and MOV-Sreg classification routes plus the
observer install/capture/publish lifecycle. The 80286 route is unchanged;
S4's evidence and corpus are 80386-only.

## Verification

- `core-machine-instruction-timing-ledger-smoke` proves exact deterministic
  ticks, physical-gate admission for the three forms, and rejection of the
  memory-source MOV-Sreg form before elapsed/provider publication.
- `core-machine-retirement-observation-s3-smoke` retains copied observation,
  lifecycle rejection and unallocated prepublication behavior.
- The explicit BYOB capture tool builds; contained preboot replay returns
  `count=64 classified=64 unallocated=0 status=0`.
- The complete configured current gate and specialized gates pass; documentation
  governance passes after the S4 evidence/index update.
- The rebuilt developer artifact is `vm-0-5-0390`, SHA-256
  `8618F97B6987A4A0D12D932A78346F7F72DD4F1E07D2A7F1188D35889AB271ED`.

## Corrective P2: First-Unallocated Stop

Coordinator review rejected P1 because the capture tool returned failure for an
unallocated observation only after continuing through its 64-retirement budget.
P2 makes `capture.unallocated == 0` a loop precondition, so the first copied
source-unallocated observation is the terminal capture checkpoint.

A project-owned temporary synthetic ROM pair places one known unallocated
prefix form at reset. The tool reports exactly `count=1 classified=0
unallocated=1 status=0`, exits nonzero, and deletes the synthetic inputs and
output. The retained S3 observation smoke continues to prove that the copied
unallocated record is published before physical rejection and before elapsed
publication. The normal owner-managed preboot replay remains `64/64`
classified and exits successfully.

## Transfer

This is a preboot prefix, not the S2 corpus completion checkpoint. The selected
full corpus runs through first boot-sector control transfer and needs
owner-managed boot media, bounded media attachment and a larger contained
observer run. S4 does not enable a physical Model-40 profile, establish a
CPU-to-board ratio, or make a board/device/L3 claim. A later T390 continuation
must extend the runner to the S2 checkpoint, retain normalized form/context
coverage for new success, and reject or transfer the first underdetermined form
before physical retirement may be selected.