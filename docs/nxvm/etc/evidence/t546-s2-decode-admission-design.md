# T546 S2 Runtime Decode And Admission Design

## Status And Baseline

Read-only source reconciliation against accepted S1 `55348a583`, 2026-10-05.
This record is the concrete review input, not implementation or CPU qualification.
Current owns admission. Shared source, tests, manifests and artifacts are unchanged.
The complete T544 receiver remains in the [task ledger](../../history/M5-T546-cpu-audit-gap-repair.md).

Owner subsequently approved this concrete scheme on 2026-10-05, including the
286 UD source disposition. The inventory below describes the pre-change
baseline; implementation and direct proof are recorded separately, not implied
by approval. The 186 authority boundary remains explicit.

## Original Sources And Decisions

Originals remain in the external CPU manual archive; no PDF or derived image
is added to the repository. Previously acquired originals were re-used.

| Original | SHA-256 | Relevant original page and disposition |
| --- | --- | --- |
| Intel 80286/80287 Programmer's Reference, 1987 | AD487BA99B48CD9F61B14C0FE912A04C7CDB4C7C14A18419AA9FAF62D8962460 | PDF 328, C-2 item 10: ten-byte limit, exception 6, no 8086/8088 limit. PDF 217, B-9 item 5 instead lists excessive length under GP. Both are genuine printed text, not an OCR substitution. |
| Intel 80286 Programmer's Reference, 1985 | 9C6067E777AE694D5F71D8ADE23014558BE4E9D156041C778AED84C1246D8538 | PDF 162, 9-10 section 9.6.1 explicitly assigns excessive prefixes/length to interrupt 6; its compatibility appendix agrees. Recommend UD from the specific exception discussion and the two agreeing appendices; retain the conflicting B-9 statement rather than claim unanimity. Owner disposition is pending. |
| Intel 80386DX Programmer's Reference, 1990 | 9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1 | PDF 302, 15-12 items 6/8: fifteen-byte GP limit and no 8086-style sequential wrap past FFFF. PDF 224, 9-20: fault classification and null error code. |

The 1985 8086/8088/80186/80188 manual does not yield an explicit 80186
whole-instruction limit in the selected searches. This is missing source
proof, not proof of unlimited length. Do not apply a 286/386 limit to 186
without authority; preserve the existing runtime policy pending corroboration.
This unresolved classification cannot be accepted as complete family proof.

Additional archived Intel 1984 Part I/II material was extracted and searched for
instruction-length and redundant-prefix limits; it supplies no explicit limit
in those searches. This negative text search is not a hardware conclusion.
Original hashes are respectively
`2D722F75216437841B23A8EFF0C7C9356E9893FD2BFF056AA6AE3F2DDAFFDC46`
and `8987ACD4F9BF204FF1A77281F38AB4A0361E56C65C4C31E19B9489121274CBCC`.
386 section 9.4/Table 9-2 defines instruction-boundary exception/interrupt
classes, not priority between two operand-validation faults. It cannot be used
as proof of CPL-versus-effective-address/page-fault priority for LLDT/LTR.

The new 1985 286 page and conflicting 1987 B-9 page were rendered and inspected.
The 286 C-2 and 386 length/fetch/GP pages were visually checked in the preceding
S2 research. Scratch renderings stay ignored and are retained for this review.

## Current Code: Complete Direct Decode Call Inventory

Search: `rg -n '^\s*_d_[a-z_]+\(' src/x86/chips/cpu/cpu_instructions.c`.
All thirteen hits match the T544 inventory. These are unchecked call sites,
not a claim that thirteen independent runtime failures were reproduced.

| Handler | Line at baseline | Required checked operation |
| --- | --- | --- |
| TEST_RM32_R32 | 11838 | `_d_modrm(2, 2)` |
| XCHG_RM32_R32 | 11877 | `_d_modrm(2, 2)` |
| MOV_RM32_R32 | 11914 | `_d_modrm(2, 2)` |
| MOV_R32_RM32 | 11949 | `_d_modrm(2, 2)` |
| LEA_R32_M32 | 11998 | `_d_modrm_ea(2, 2)` |
| CALL_PTR16_32 | 12509 | `_d_imm(4)` |
| RET_I16 | 14070 | `_d_imm(2)` |
| RETF_I16 | 14581 | `_d_imm(2)` |
| INT_I8 | 14630 | `_d_imm(1)` |
| AAM | 15031 | `_d_imm(1)` |
| AAD | 15059 | `_d_imm(1)` |
| JCXZ_REL8 | 15170 | `_d_imm(1)` |
| JMP_REL32 | 15321 | `_d_imm(2)` |

The existing CHECK_RETURN macro executes its argument before inspecting the
exception. A later checked operand read therefore does not guard an earlier
failed decode. Wrap the actual decode, before dependent memory/stack/transfer
or arithmetic work. Test first ModRM, later displacement and immediate failures
separately, including no dependent provider effects and first-fault preservation.

## Shared Mechanism And Interdependent Boundaries

- `_s_read_cs -> _kdf_code -> _kdf_skip` is the existing runtime fetch/consume
  owner. The opcode and group lookahead also call `_s_read_cs`; they must use
  the same byte-budget decision, including cached bytes. The 15-byte observation
  array and `oplen` are not runtime enforcement.
- There are 224 direct `cpu_state.data.ip++` sites. They bypass `_kdf_skip` in
  below-386 handlers. A budget or cursor repair only in `_kdf_skip` would leave
  a second consumption rule. Reconcile these with the existing `_adv` pattern,
  not another counter or parser. Preserve generation-specific sequential rules.
- `ExecInit` speculatively reads its entire prefetch window. It caps at the CS
  limit but not a paging boundary. An otherwise complete one-byte instruction
  can fault on an unneeded following page. Limit speculative fill to a safe
  contiguous boundary; required bytes use the existing checked fetch owner.
- `ExecIns` currently calls breakpoint/decode processing even after init has
  recorded an exception. Finalize that failure before dependent processing.
- Post-body `_s_test_eip` checks the next instruction's cursor and can roll back
  an already completed instruction. `_s_test_esp` likewise checks an unused
  pointer. Both have only the ExecIns callers. The proposed S2 patch includes
  removing these generic postchecks, while retaining actual branch, return and
  stack-access checks. Planned S4 still independently qualifies every retained
  transfer/frame caller and publication context; it is not silently cancelled.
- Preview copies CPU/instruction state, enables `preview_mode`, then scans a
  fixed 15-byte zero-padded observation. Page-bounded fill must not make missing
  bytes appear to be zeros. Use only actually captured bytes; preserve unavailable
  when a complete lexeme cannot be proven. Do not execute handlers during preview
  or turn its diagnostic scanner into a second runtime decoder.
- The final `1` argument of `_kma_read_logical` is `force`, not observe-only.
  Bus observation and page A/D suppression use `context->preview_mode`.
  There is no proven runtime observe-only bug from that argument. Full implicit
  paging/A-D precedence remains S13, not falsely claimed fixed by this design.

## Admission Reconciliation

Initial inspection considered LGDT/LIDT's early check complete. The subsequent
cross-mode matrix disproves that reading: `_IsProtected` excludes VM86, so
its guarded VM predicate cannot reject VM operands. The implementation uses
the existing CPL definition before operand reads, as recorded in the
[implementation evidence](t546-s2-decode-admission-implementation.md).
LLDT/LTR still read the r/m operand before `_s_load_ldtr/_s_load_tr` checks CPL.
Their real/VM exclusion already exists because `_IsProtected` excludes VM.
Place the applicable privilege admission before operand access; keep descriptor
validation and busy publication with the existing load owner. The 386 source
instruction pages are 17-97/98, 17-102 and 17-113. Merely listing exceptions
does not establish every simultaneous-fault priority; test/source claims must
identify the applicable precedence authority, not infer it from list order.

Generation, group, segment-register and memory-only admission are inspected
through the existing metadata, `_d_modrm_*`, 0F dispatch and handler owners.
The scanner rejects LOCK for observation; that is not runtime illegality proof.
Do not change LOCK/NPX bus semantics here; S15 retains their complete receiver.
No new blanket rejection may replace a valid historical-generation form.

## Proposed Existing-Style Control Flow

Illustrative pseudocode, not a new API or a line-for-line implementation:

```c
ExecInit(context);
if (instruction_state.data.except) {
    ExecFinal(context);
    return;
}
/* Existing table dispatch; every required code read checks its family budget. */
CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
/* Only after successful decode: existing operand operation. */
CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
/* No generic validation of an unused next EIP/ESP after the body. */
ExecFinal(context);
```

Budget decision: before reading required bytes, compare the existing start and
current code cursor plus requested width against the proven family maximum;
286 ten/UD subject to source disposition, 386 fifteen/GP0. Do not count a repeated
lookahead twice. Do not add a mirrored architectural instruction counter.
The exact cursor handling must prove legal endpoints and branches before reuse
of receipt fields is accepted. Early-family diagnostic capture bounds are not
an invented hardware runtime length limit.

## Implementation Review And Exit Evidence

Proposed production change remains CPU-local, chiefly `cpu_instructions.c`;
no public API, Lib/Common/MyNES or App BIOS/profile fix. Mechanical direct-IP
substitutions preserve the original table bodies; final diff size must be
counted after implementation rather than advertised as a thirteen-line fix.
Owner-local CPU tests cover family/mode/size/prefix/ModRM/SIB/disp/immediate,
cached/required/page-end fetch, init/caller failure, legal endpoint retirement
then next-fetch failure, privileged operand rejection and preview isolation.
Include register/stack/memory/port effects, original exception identity and
restart PC, not only final failure bits. Regression data stays in test code.

Existing `support/cpu_bus_fixture.h` has a global `fail_transfer`; enabling it
before refresh fails initial prefetch rather than a selected later decode.
The intended regression must distinguish prefetch, required fetch and data
provenance and fail a selected required byte/address after successful opcode
admission. Its 2048-byte modulo backing alone cannot prove real cross-page or
FFFF boundary behavior; use the existing paging fixture where translation is
the subject. These are planned owner-local test changes, not executed probes.

After concrete Shared approval: implementation, complete units both widths,
manifest/Types/corpus gates, all eight affected 0546 PC EXEs and original
receiving boot contexts once. Preserve owner INIs/media and exact MyNES pair.
No runtime test or implementation result is claimed in this design record.
The whole S2 batch cannot close while source disposition, form/privilege
precedence or a necessary endpoint remains unqualified. Original S3/S4/S13/
S14/S17 receivers stay in the task ledger for their full later proof.
