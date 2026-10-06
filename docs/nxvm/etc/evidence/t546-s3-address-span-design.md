# T546 S3 Effective Address And Segment-Span Design

## Baseline And Review Boundary

Continuation of the owner-approved complete CPU repair, after accepted S2
c3c9fd550. The source/design stage makes no implementation changes. On
2026-10-06 the owner then explicitly includes x86/chips, x86/core and
ibmpc in automatic approval; only the nonexistent x86/devices is excluded.
This authorizes the existing-owner scheme and
its matching verification. No Lib/Common/MyNES or new public API is admitted.
The original T544 family findings remain incorporated by the T546 ledger.

Architecture/coding governance keeps effective-address construction distinct
from operand-span validation and from physical transfers. There is one CPU
access owner; App/profile, Lib and Common do not acquire CPU repair policy.

## Original Sources Inspected

Existing owner archive originals are rehashed and inspected on 2026-10-06;
no new acquisition, third-party source import or protected-file publication.

- Intel 8086/8088 User's Manual (1981), SHA-256
  3EEA6CA77AD4046AE7ADE731410793206EEBE8EC9A3F8AE75895685D38F4FFE5:
  PDF 31/2-12 defines unsigned 16-bit offsets and modulo-64K addressing;
  PDF 51/2-32 describes BX plus unsigned AL and the two-word far pointer.
- Intel 80286/80287 Programmer's Reference (1987), SHA-256
  AD487BA99B48CD9F61B14C0FE912A04C7C14A18419AA9FAF62D8962460:
  PDF 188/11-2 strictly excludes offsets at/below an expand-down limit and
  explicitly makes FFFF an empty segment. PDF 270/B-62 specifies real-mode
  rejection of an overflowing LDS/LES pointer. Do not infer legality for an
  omitted even endpoint from this abbreviated example list.
- Intel 8086/8088/80186/80188 User's Manual (1985), SHA-256
  2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF:
  PDF 25/1-9 repeats legacy modulo-64K addressing; PDF 41/1-25 identifies
  BOUND's two transfers, supporting retention of the two scalar phases.
- Intel386 DX Programmer's Reference (1990), SHA-256
  9A8188F9D2282B113FC421E225CC2A643FCDC349E5C3C43659BD2CF6620F1EA1:
  PDF 153/6-5 gives complete-span limit rules and strict expand-down bounds;
  PDF 284/14-6 distinguishes early wrapping from 386 GP/SS limit faults;
  PDF 501/17-183 defines XLAT address-size selection; PDF 350-351/17-32-33
  distinguishes an immediate bit within a word/dword from displacement of
  a memory bit string. PDF 282/14-4 explicitly retains loaded descriptors
  when segment registers are not reloaded on leaving protected mode. These
  pages are rendered and visually inspected.

Bochs 2.6 is read-only corroboration, not copied source or a hardware oracle:
cpu/bit16.cc SHA-256
A965D19506D4F589785D217F50CA33CC3BEF11D486EEDEFF86EBB861538FE55E and
cpu/bit32.cc SHA-256
A3B1FA35B902C5CD8B7E1E126997C3D1FFCC5E40247E4C8F975423D52AE8C2C5.
Its memory-immediate BT/BTS resolves the original EA and masks the immediate
within the word/dword, without adding another memory displacement. This
corroborates the original manual; it does not upgrade an emulator-only model.

## Complete Existing-Owner Inventory

Searches: `rg 'mrm.offset \+=' cpu_instructions.c`, the complete XLAT and
_d_bit_rmimm bodies, _kdf_modrm/_d_moffs/index formation, and logical
read/write/test/test_access -> _kma_linear_logical. Inspect the matching
operand/address, bit-test, far-load, protected-data and paging test owners.

| Class | Present defect or disposition | Proposed owner repair |
| --- | --- | --- |
| XLAT | Two word-offset BX+AL branches are unmasked; dword addition is separate. | Mask only the two 16-bit EA expressions. Preserve 32-bit addressing and segment overrides. |
| Bit-string EA | Four register-index additions can leave a 16-bit EA outside its width; two immediate additions wrongly move the memory address. | Remove immediate-driven displacement, normalize register-derived EA by actual address size, and widen negative dword displacement arithmetic before subtracting. Keep the one helper for all four operations. |
| Real/VM/cache data span | A nonprotected DATA branch unconditionally substitutes FFFFFFFF, including 286 and VM86. | Remove the widening; use the actual cached limit. Preserve separately evidenced extended-cache behavior, never force all caches to FFFF or mask all logical addresses. |
| Empty expand-down | Both DATA/STACK compute limit+1 in 32 bits; FFFFFFFF becomes zero. Legal non-big 286 bounds already work. | Widen the lower bound before addition in the existing range owner. Keep the subtraction-based full-span upper check. |
| Multi-field operand | Fifteen secondary-offset additions: BOUND two, LES/LDS four, indirect far CALL/JMP four, LGDT/LIDT two, LSS/LFS/LGS three. Fields are read before the full segment span is known valid. | One owner-local paired-read preparation using existing logical span validation, followed by the original two physical read phases and packed local result. Normalize the second offset only for source-qualified early wrapping. Remove repeated caller offset mutation/read glue. |
| Fault class | Generic nonprotected 386 SS failures currently raise SHUTDOWN, even for ordinary SS-selected reads. | Reconcile against the explicit 14-6 SS contract; ordinary span rejection must not itself mean CPU shutdown. PUSH/PUSHA special admission and failed delivery remain their S6/S9 dependencies, not a new blanket stack rule. |

Ordinary 16-bit ModR/M formation already masks its completed sums; MOFFS
decodes the selected width and string indices have their width-specific owner.
Keep their valid paths and prove boundary receivers instead of adding another
global mask. Instruction fetch remains S2's accepted owner.

The six bit-offset additions and fifteen composite additions are different
contracts. EA reduction must not wrap a later CPU's overflowing operand span.
The immediate bit tests currently expect neighboring words to change for raw
10h/21h immediates; these are contradicted oracles, not independent sources.
Correct them after Shared review and test all four groups and both widths.

## Minimal Existing-Style Scheme To Review

Illustrative pseudocode, not new public interfaces:

```c
/* XLAT word EA, not a global logical-address truncation. */
offset = X86_CPU_MASK_U16(cpu_state.data.bx + cpu_state.data.al);

/* Existing range owner: the empty 32-bit interval remains representable. */
lib_u64 lower;
lower = (lib_u64)rsreg->limit + 1u;

/* Existing-style internal paired read, shared by pointer/bounds/table users. */
wraps = _kma_real_legacy_segment_wrap(context, start,
    first_bytes + second_bytes);
CPU_TRACE_CHECK_RETURN(_m_test_logical(context, segment, start,
    first_bytes + second_bytes, 0));
CPU_TRACE_CHECK_RETURN(_m_read_rm(context, first_bytes));
low = instruction_state.data.crm;
instruction_state.data.mrm.offset += first_bytes;
if (wraps)
    instruction_state.data.mrm.offset =
        X86_CPU_MASK_U16(instruction_state.data.mrm.offset);
CPU_TRACE_CHECK_RETURN(_m_read_rm(context, second_bytes));
instruction_state.data.crm = low |
    (instruction_state.data.crm << (first_bytes * 8u));
/* Callers decode copied fields only after success; publish as before. */
```

Do not collapse two scalar bus phases into one large provider access: that
could alter transfer/alignment waits. Logical preflight does not read MMIO,
translate pages or publish A/D. Complete paging preparation remains S13.
There is no generic undo log, second parser, cached segment mirror or API.
The dword negative bit-index arithmetic dependency belongs here because EA
extreme cases cannot be defined while the existing subtraction overflows;
the rest of S5's arithmetic universe remains intact.

The sweep also records `_d_bit_rmimm`'s unused `write` argument. Do not
mistake it for proof of RMW write-permission preparation. The complete
read/modify/write segment/page admission versus accepted bus effects is a
retained S13/S15 receiver across ALU and bit users, not just this helper.
This design qualifies EA and full segment-span geometry, not all RMW
failure atomicity or irreversible-provider rollback.

## Proof And Dependency Gates

Before implementation, reconcile 186/286/386 family pages for each changed
form, cached-limit transitions and real/VM SS exception selection. Preserve
the explicit general chapter versus abbreviated instruction-dictionary
distinction rather than silently treating every stack failure as shutdown.
Do not declare PUSH/PUSHA or exception delivery complete from a span test.

For concrete review, use the explicit SS-selected operand rule in 14-6,
rather than the instruction dictionaries' abbreviated all-operand interrupt-13
sentence. Remove the generic nonprotected-386 SHUTDOWN branch; retain the
286 real-mode GP policy and ordinary SS fault selection. A real-mode MOV
through SS at FFFF with a separately valid stack pointer illustrates why an
ordinary reference can fault and deliver without directly shutting down.
The 14-8 stack-operation shutdown paragraph is not authority to terminate
every SS-selected operand. Special push admission and failed delivery still
require their full S6/S9 source/context matrix.

Bit-index review also includes two unsigned bit-mask literals and widening
the negative dword subtraction before division. These are necessary defined
arithmetic for this helper's extreme EA cases, not a claim to finish S5.

Repository-only matrices distinguish bytes at offset zero/10000, 16/32
address and operand sizes, overrides, positive/negative/extreme bit indices,
raw high immediates, 286/386 normal/expand-down endpoints, empty ranges,
read/write/preflight and all composite fields. Invalid complete segment spans
must reject before any operand provider read/write; valid early wrapping keeps
its transfer order. Provider failure keeps the original first fault, CPU
rollback and accepted external effects, not fictional MMIO rollback.

Frames/tasks/descriptors use the same range owner, but their full layout,
admission/publication and delivery contracts remain S6/S9-S13. S3 must map
each caller to repaired geometry, source-proven non-applicability or that
explicit retained dependency; a later checkpoint cannot qualify it silently.

After concrete Shared review: preserve original table handlers, complete both
widths' full units, manifests/corpus/Types/specialized gates, rebuild all eight
affected 0546 PC artifacts and run the 58 original external contexts once per
successful final-source group. No MyNES rebuild, INI/media change or timing
grade downgrade. No new L1 or unresolved source policy is silently admitted.
