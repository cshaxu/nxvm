# T2 S8 Native Memory Transfers

Baseline `0082edaae`. This batch consumes the V14 native memory-address and
physical-transfer subfamily, not full V14 or V30 qualification. The old FFFF
data-word rejection is replaced at the shared CPU memory owner, not by an
INS/EXT special case.

## Source Review And Model Grade

The pinned October 1986 NEC manual, printed 3-5 and 4-1--4-3, provides the
segment/address roles, 20-bit memory bus and byte/word lane rules. PDF 20--22
were rendered and inspected. Even words use one word cycle; odd words use
two consecutive byte cycles. Native fetch at an odd branch target starts with
a byte, then words. There is no native protected-segment fault mechanism.
The semantic prefetch cache still is not a cycle-accurate queue/timing model.

The exact data-word behavior at the segment tail uses a source-backed L2
model: the starting effective address is formed by the native instruction,
then the operand span continues physically, modulo the 20-bit bus. This is
distinct from 16-bit instruction-pointer wrap. It is not a claim of direct
original-V30 silicon measurement. Coordinator review selected this model
before implementation within the approved V30 work, without a public-boundary
change or copied implementation.

Read-only official MAME references are pinned at
`8a18b751b388390db32812be43b8802de8cac839`:

- [necpriv.ipp](https://github.com/mamedev/mame/blob/8a18b751b388390db32812be43b8802de8cac839/src/devices/cpu/nec/necpriv.ipp),
  5477 UTF-8 bytes, SHA-256
  `43EF08521B1A4A0605974325F39CBC277E1D2807706857C29B06B5751E596135`.
- [nec.cpp](https://github.com/mamedev/mame/blob/8a18b751b388390db32812be43b8802de8cac839/src/devices/cpu/nec/nec.cpp),
  18663 UTF-8 bytes, SHA-256
  `B27B4CE95E43AF3F8D603BE67751BDA84B38EF7457BE40C33F2C8E17CFC5C2C9`.

These declare BSD-3-Clause/Bryan McPhail. Their V30 word access is a linear
unaligned memory operation and its program address space is 20 bits. They
were inspected only as behavioral evidence, not saved/imported, translated,
linked or copied into this MIT source. Native implementation is independently
written around the existing logical/physical/bus pipeline and manual facts.
This reference does not promote I/O, timing, silicon flags or other models.

## Sole Production Path

The existing logical-address owner bypasses protected descriptor checks only
for native V30 code/stack/data segment roles. Native data spans therefore do
not invent GP/SS exceptions or inherit early-Intel data wrapping. The existing
instruction-fetch provenance selects 16-bit IP wrapping at the common span
owner. CS-overridden data remains data, not instruction fetch. Paired operands
reuse their original scalar sequence and contiguous operand span.

One private bus-part predicate owns the V30 byte/word lane decision for both
physical read and write. Each chunk returns through the original bus callback
and BEGIN/COMMIT/CANCEL publication; every address is masked to 20 bits.
Read and write retain separate accepted-effect ownership. There is no new
memory callback, public type/API/layout, persistent state, second memory or
instruction path, journal, replay thread or clock. Old Intel branches remain.
The six-byte semantic prefetch cache fills five bytes on an odd start so its
requests are byte-then-words, without adding a second prefetch state owner.
Electrical arbitration, full queue overlap and clocks remain V14/V15 work.

## Owned Tests And Failure Boundary

The new original ROM-free CPU fixture has real 1-MiB backing storage, rejecting
out-of-bus-width addresses or illegal word lanes instead of hiding them behind
a small modulo buffer. It records actual provider address/width/provenance,
observation/acceptance and existing external-cycle phases. It is a test fixture,
not production state or a new test framework.

Actual MOV byte/word loads/stores exercise all segment choices, even/odd starts
and FFFE/FFFF tails. A distinct segment-base sentinel rejects accidental
16-bit data wrap. Forced linear reads/writes cover widths 1--6, both lane
alignments and 20-bit top/wrapped starting addresses. LDS proves the complete
paired operand sequence and atomic register/segment publication. PUSH/POP
prove SP=1 tail crossing and inverse restoration. A MOV immediate spanning
the CS tail proves fetch wraps separately from data; an odd-start preview
and actual fetch prove byte/word widths and retained cache capacity semantics.

Every chunk position of a six-byte odd access is refused for read and write;
accepted buffer/memory prefixes remain and unaccepted suffixes do not change.
Actual MOV and PUSH inject both word-byte failures, preserving PC/register/SP
while retaining accepted writes. All four LDS reads fail independently without
publishing partial AX or segment state. Prefetch and observe-only failures at
each chunk preserve their contracts. No extra request/replay follows failure.
Preview changes neither the original CPU/cache nor real cycle publication.

Existing bit/nibble tests now expect two writes for an odd word, not a single
logical transfer callback. Existing bit-field tests retain their independent
bitwise oracle, restore the formerly failing FFFF-start case and check actual
last provider address as well as physical cycle count. The last-address check
prevents a small modulo backing array from hiding the wrong wrap direction.
All original semantic/flag/failure assertions remain.

The similar-issue sweep covers all logical/preflight/pair/stack, fetch/prefetch,
observational and physical read/write paths in the CPU. Searches include
`_kma_real_legacy_segment_wrap`, `_kma_linear_logical`, `_kma_*_physical`,
`_m_read_pair` and every V30 fixture's read/write-count assertion. Hardware
lane differences remain at physical ownership; no instruction gets a bus
workaround. I/O lane/restart, full HLT/WAIT/LOCK and timing/mode contexts retain
their existing V09/V14/V15 receivers rather than being silently qualified.

## Verification And Delivery

Shared `70dde1d31` and eight NXVM artifacts `0ed0a9e30` are pushed. Both
original Release Ninja/ccache graphs complete full builds. First complete
repo-only unit runs pass x64 661/661 in 310.12 seconds and x86 661/661 in
180.98 seconds, including all 172 X86 Core units. No retry, budget increase or
assertion deletion is used. Manifests and registered Types, inward dependency,
ownership and negative gates are included. Both widths' six V30 units also
pass after relinking the changed CPU and affected assertions.

Both original graphs link/deploy the four receiving PC profiles, reusing
production objects, then restore default. The [artifact inventory](t2-s8-pc-artifacts.json)
records all eight PE widths, product/version/INI tokens and hashes at retained
revision 0563. MyNES pair, all INIs/media and other Shared packages remain
unchanged. No external-ROM integration, desktop/RDP, physical audio, board/game
qualification or MyRaidenII executable is claimed by this batch.

Actual source/test deltas from `git diff --numstat`, excluding manifests/docs/
artifacts: production +45/-1/net +44; tests +335/-4/net +331 (326-line new unit,
registration and necessary bit/nibble/field bus assertions); combined
+380/-5/net +375. New code has real native address/lane responsibility and
owned proof; no framework or alternate instruction memory route is added.
Source SHA-256:
`296EF93EC09FE827817798286B1F07C13E4C6B316939F31517F20234D87F99A9`.
The historical instruction-file warning exception remains; a separate
`-Wall -Wextra -Wpedantic` syntax review finds no warning in the changed
regions. It does not claim all legacy source builds without exceptions.

Ignored full-build/unit/focused/artifact logs are retained under
`build/myraidenii/t2-s8-*` for final T2 review. Three owned manual render PNGs
were removed after inspection; pinned manual/text remain. Documentation,
relative links, exact manifest/artifact identities and whitespace are checked
before delivery. Coordinator actual-change review must reconcile this native
memory batch against the full ledger, retaining the explicit L2 model and
still-unqualified I/O, electrical/timing, mode and board contexts.
