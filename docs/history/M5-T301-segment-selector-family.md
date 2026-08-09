# M5 T301: Segment Selector Family

## Implementation Record

T301 implements the admitted 80386 segmentation and selector-instruction
family in the single core CPU executor: `MOV`/`POP` segment loads,
`LDS`/`LES`/`LFS`/`LGS`/`LSS`, `ARPL`, `LAR`, `LSL`, `VERR`, and `VERW`.
It neither adds a second executor or state owner nor changes Console,
debugger, boot, or media behavior. The active S4 packet and current artifact
remain authoritative in [STATUS.md](../STATUS.md); the coordinator independently
accepted the implementation evidence and closed the task.

## Reference Conclusion

Intel's *80386 Programmer's Reference Manual* (1986), Chapters 3, 5, 6, 14,
and 17, is the semantic authority. Read-only comparisons used Bochs 2.6
`cpu/protect_ctrl.cc`, `cpu/segment_ctrl.cc`, and `cpu/fetchdecode.cc`, plus
PCjs 2.00.0 `machines/pcx86/modules/v2/x86ops.js`, `x86op0f.js`, and
`segx86.js`. They confirmed, without source transfer, the project matrix:

- `ARPL`, `LAR`, `LSL`, `VERR`, and `VERW` are protected-mode selector checks
  and reject outside protected mode; `MOV`/`POP` and LxS retain their legal
  real-mode segment-load forms.
- LxS forms have a memory-only second operand. The selector remains 16-bit;
  operand size controls only destination and pointer offset width, while `POP`
  segment forms use the matching stack transfer width.
- FS/GS require 80386. Reserved and later-CPU forms remain `#UD`.
- Query instructions return failure through ZF, including non-present or
  privilege-inaccessible descriptors, without a selector protection fault.

## S4 Corrections And Focused Probes

The S4 audit fixed two selector-query omissions in
`src/core/machine/cpu_instructions.c`:

1. `LAR`/`LSL`/`VERR`/`VERW` now clear ZF for non-present descriptors.
2. `LAR`/`LSL` now apply DPL/RPL visibility checks to admitted system
   descriptors before exposing access rights or limits.

The focused synthetic probe
`tests/machine/core_machine_segment_selector_smoke.c` adds non-present query
checks, available-386-TSS visibility cases, LSS 80286 rejection, and all-five
LxS non-present atomicity cases. Those cases verify the specified exception,
unchanged general/stack/flag/segment-cache state, and unchanged descriptor
access byte before the failed load commits. The retained ARPL probe remains a
separate regression.

The similar-issue sweep covered selector/descriptor handlers, 16/32-bit forms,
profile gates, and validation/commit paths in `src`, `tests`, and CMake. The
only production hits were the non-present and system-descriptor visibility
rules above, both fixed. LDT breadth, task gates, virtual-8086, task switching,
and later system forms remain the explicitly bounded Queue admission, not
untracked defects.

## Verification

- `cmake --build --preset current-gcc` rebuilt
  `build/output/nxvm_0_5_0301.exe`.
- SHA-256:
  `54D6BEB2BEF9E495C09DD4AA976288449EB0608567A2F32F61693219FFA23E3D`.
- `core-machine-segment-selector-smoke` emitted
  `M5:T301:SEGMENT-SELECTOR:OK`.
- The retained ARPL probe emitted `M5:T263:S2:ARPL:OK` and
  `M5:T263:S3:ARPL:CORPUS:OK`.
- `cmake --build --preset current-gates-gcc` passed 51 static/governance
  targets and 130/130 CTests, including documentation governance.
- `git diff --check` passed with the final S4 tree before coordinator review.

The S4 non-functional gate repair changed
`cmake/verify_request_transport_closure.cmake` to read the existing detailed
machine contract record rather than a removed contract path. Its ingress and
machine-stop assertions are unchanged.

## Setup Observation Limit And Deferral

One bounded product-window observation budget was attempted with owner-supplied
external media. Host-side window discovery stopped before any guest command
was sent; no guest checkpoint, stdout/stderr, or NXVM process residue was
obtained. An initial host automation attempt also stopped before guest input.
Neither attempt is Setup progress or a product regression.

The sole deferred verification is an owner-controlled manual Setup observation
using the retained product path. It does not alter the T301 CPU result, create
a repair task, or introduce guest media, local paths, or traces into the
repository.
