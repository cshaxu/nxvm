# T582 S37 — memory and bus hot paths

## Cache-fill proof with unrelated aliases

`memory_query_cpu_cache` already rejects a mapping intersecting the 16-byte
candidate line. The later `memory_is_plain_span` rejects any nonempty mapping
list, including mappings just proven disjoint, and therefore falls back to
bytewise route queries. A more selective proof might avoid that repeated work
when address decoding is unchanged and all providers are also proven
disjoint; any implementation would still have to establish all alias, raw
provider, span, backing and failure semantics. This is an identified possible
general improvement, not an admitted code change.

A scratch-only counter instrumented the actual x64 PC110 cache-fill query at
the private fixed startup checkpoint. It recorded 2,566,175 fill probes and
zero with a nonempty mapping list or address-clear mask; thus zero reached the
disjoint-mapping opportunity. The run completed ten million instructions in
20,454 ms, at 79,964,294 ticks and PC `0003dbc5`. Every query was instrumented,
so this is a frequency observation rather than an unperturbed performance
measurement. It does not establish Game-stage behavior, x86 behavior or other
machine configurations. It does not justify changing production for the
current PC110 startup objective. No source, media or release EXE was changed.
Log SHA-256:
`1E6B586016FDE001B25423E225402C981B61E63F70533B5E5B0025B35B54B580`.

The first scratch compile failed because the diagnostic declaration preceded
the Types include; it produced no measurement. The fixture was corrected and
the completed run above used the normal execution path.

## Multi-byte ordinary-RAM span frequency and sampled cost

The ordinary RAM path resolves its provider first and, for multi-byte access,
separately checks whether the whole span is free of address masking, aliases
and intersecting providers. A scratch counter on the fixed x64 PC110 startup
checkpoint observed 133,126 multi-byte ordinary-RAM reads, 133,124 with a
plain span; and 5,289,464 writes, 5,273,080 with a plain span. These counts
include the stopping interval and duplicate the proof for measurement, so
elapsed time is not a baseline measurement. The large write count reflects
the startup workload and is not a Game-stage count. Log SHA-256:
`8C82C8D43BAE9F9FF16797EAF2574180193F0A15EE7A3B585A7179C6DABFEEB4`.

A second scratch build sampled the existing span proof one time in 256 without
duplicating its result. It recorded 5,422,534 calls, 21,181 samples and
1,576,280 accumulated host TSC units, roughly 74 units per sampled call. It
reached the same ten-million-instruction / 79,964,294-tick / `0003dbc5`
checkpoint in 18,647 ms. Deterministic sampling, non-serialized TSC and the
extra instrumentation limit interpretation; no host TSC-frequency conversion
or precise whole-program percentage is claimed. The proof is frequent but
small, and folding its partial-overlap checks into route selection would also
add work to routes that do not reach ordinary RAM. Do not replace routing or
add a provider-bound cache merely from call count. Game, x86 and complete
provider-state behavior remain unmeasured. No production code or asset changed.
Log SHA-256:
`655375B4F6A48F1CEC519D02B521242CC3519F395D745BC21DE0E636BBC9A276`.

## CPU write snoop frequency

An x64 task-only wrapper sampled the existing Machine CPU-write snoop callback
at the same fixed PC110 startup checkpoint. It counted 6,634,436 calls; all
matched the existing CPU-owned transaction/range check that returns without
invalidating the CPU's own write. One in 256 produced 25,915 intervals with
1,563,830 accumulated host TSC units, roughly 60 units per sampled call. The
run reached ten million instructions, 79,964,294 ticks and PC `0003dbc5` in
19,996 ms. Counts include instrumentation and the stopping interval; TSC is
not serialized and no exact whole-program percentage or Game/x86 conclusion
is supported.

This does not make `cpu_access` a safe substitute for transaction ownership:
debug and firmware writes also pass it, while aliases and provider side effects
may require snoops at a different physical address. Eliminating a callback
would require explicitly preserving the exact transaction range and all those
owners. The measurement does not justify propagating a new suppression flag
or weakening invalidation as a PC110 usability fix. No production code or
artifact changed. The first scratch command failed to write the script because
the host PowerShell lacked a requested parameter; it was corrected before the
completed run. Log SHA-256:
`E9BD39568FFF8E1D10675F917BD4799D52BDD0A7EF29EA5E2D6E93BFB97DFAE9`.

## Game-drawing instruction execution samples

An x64 scratch build sampled one in 256 calls to the existing `ExecIns`,
starting immediately before the Machine input that selects Game and stopping
when the full Game frame became stable. The sample records the final decoded
primary opcode after the call and its inclusive host TSC interval; exceptions
or record reuse can affect classification. It does not time the separate
timing selector, Machine scheduler, or a complete instruction retirement.

The process exited normally in 271.03 seconds; Enter was sent at sample 233
and the complete frame appeared 33 one-second samples later. The stable frame
SHA-256 was `E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.
Across 88,032 sampled calls and 74,803,282 accumulated TSC units, the leading
sampled primary opcodes were:

| Opcode | Samples | Accumulated TSC units | Sampled execution share |
| --- | ---: | ---: | ---: |
| `8A` (MOV byte) | 7,072 | 7,501,180 | 10.0% |
| `D0` (byte rotate/shift group) | 7,687 | 5,171,924 | 6.9% |
| `80` (byte arithmetic group) | 6,595 | 4,841,119 | 6.5% |
| `74` (JZ) | 7,350 | 3,933,880 | 5.3% |
| `84` (TEST byte) | 4,893 | 3,824,670 | 5.1% |
| `75` (JNZ) | 7,008 | 3,790,140 | 5.1% |

These percentages are within the sampled `ExecIns` intervals only, not the
whole Machine. Sampling is deterministic and can alias periodic guest loops;
TSC is non-serialized, elapsed intervals include instrumentation and scheduling,
and the stage boundary crosses host and worker threads. The frame is not proof
of identical complete guest state, native Window/RDP responsiveness or playable
input. No product source, owner media or EXE changed. The instruction samples
direct investigation to `8A`'s existing operand decode and read path, while
preserving all other opcodes and fault semantics. Guest log SHA-256:
`8831B1CF4FA9E141BA82A3E61979EF5C72EBDB5688D655C89A0662E74575425F`.

### MOV destination-read cost

`MOV_R8_RM8` decodes a ModR/M byte, reads the source operand and writes the
destination; the shared decoder also reads the destination register's old
value into its temporary before the MOV overwrites that register. A second
private x64 Game run timed that existing read inside the same one-in-256
`ExecIns` samples. It completed in 265.25 seconds; Enter was sent at sample
229, a complete frame appeared 31 samples later, and the saved frame matched
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.

There were 7,111 sampled `8A` instructions, totaling 9,627,496 host TSC units
for their full `ExecIns` intervals; their destination-old-value reads totaled
624,200 units over the same 7,111 samples. This is about 6.5% of those MOV
intervals or 0.8% of the 79,608,896 total sampled `ExecIns` units in this
window. The nested read includes an additional non-serialized timer pair and
classification overhead; it must not be added to its parent. Sampling and
stage caveats above apply, and this is neither x86 nor full guest-state proof.

Removing the read correctly would require an explicit operand-use contract in
the shared ModR/M decoder and updating its many existing callers, while
preserving NEC, fault, timing and diagnostic behavior. For `8A` alone the
measured upper bound is too small to justify that broad change as the PC110
usability fix; other instructions have not been measured by this probe. The
existing decoder is retained. No production code or artifact changed. Guest
log SHA-256:
`AAD343F916CEFB6A241646078BAAC4173385A86D9C5F9739347EA95820355CB3`.

### MOV decode, source read and destination write

A further single private x64 Game run sampled the same `8A` instruction
execution and timed its existing `_d_modrm`, `_m_read_rm` and `_kma_write_ref`
calls within each sample. It exited normally in 262.90 seconds, sent Enter at
sample 229 and produced the complete frame 29 samples later; the saved frame
matched `E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`.
All four counters recorded 7,016 samples: the outer `ExecIns` intervals totaled
9,078,060 TSC units, ModR/M decode 1,387,960, source read 2,658,760 and
destination write 388,540. These are approximately 15%, 29% and 4% of the
outer intervals. Each nested timer and report also increases the outer cost;
the remaining outer time includes other instruction execution and instrumentation.
The nested values do not include the independent timing selector or scheduler.

The source read is the largest of these three measured subintervals, but it
is not evidence that raw RAM copying is the cost: it also includes segment,
paging, cache, bus and fault processing. The `8A` samples were about ten
percent of sampled instruction-execution units in the preceding run, so even
removing an entire subinterval would not establish the required overall gain.
Deterministic sampling, non-serialized TSC, host scheduling and asynchronously
selected stages remain limitations; Game-frame identity is not complete state
or native/RDP gameplay qualification. No production source or asset changed.
Guest log SHA-256:
`00B7E4901C39B6B0B0C98052D64DD8B539C9F233A5D525B11E8FC7624EEC2543`.

## Disabled data-breakpoint scan

Every completed 386/486 instruction checks recorded memory accesses against
the four data breakpoint slots. When all DR7 local/global enable bits are
clear, no enabled comparator can cause a trap; the caller only publishes the
matched mask if an enabled comparator matched. The matcher now returns zero
after initializing that mask in this case, avoiding a scan proportional to the
recorded access count. If any slot is enabled, the original complete scan
remains, including disabled slots that match alongside an enabled slot. No
state, public interface, second executor or guest timing rule is added.

A scratch baseline/candidate/candidate/baseline comparison at ten million
instructions, 79,964,294 ticks and PC `0003dbc5` took 17,927 / 17,851 /
20,344 / 19,375 ms x64 and 22,433 / 22,709 / 21,195 / 22,509 ms x86. Both
widths change direction and no product speedup or Game-state equivalence is
established. The bounded simplification is retained under the owner's
criterion because the disabled case no longer scans each access, with no
clear consistent timing regression in these noisy samples. Log SHA-256:
`A11DCDAE258DE191A3143DF5CB63AAF7A0A7B9F62BF1C256B60D7C47AAB9B21C`.

The existing `cpu_debug_state` comparator test covers all sixteen enabled
masks, sixteen matched masks, local/global settings, instruction and data
forms on 386 and 486. The existing Debug, CPU lifecycle and cache tests passed
3/3 on each host width after the production change. Cached unit suites passed
712/712 for each width. Default integration passed 47/48 initially on each
width: `mynes.integration.native-window-smoke` reported an invalid or black
pixel at line 345 in both runs; each passed when rerun separately. The initial
failures remain a desktop-test stability limitation, not a clean full-suite
pass. The 5160, 5170 and DeskPro profile integration suites passed 3/3, 5/5
and 6/6 respectively at each width. MyRaidenII startup and product-console
receiver checks passed at both widths; they are not gameplay qualification.
Fourteen deployed executables were checked for PE width; the default NXVM,
PC110, MyNES and MyRaidenII builds matched their deployed files at both widths.
The artifact inventory is `t582-s37-data-debug-artifacts.json`.

The PC110 game workload took 267.78 seconds to reach its sampled game frame,
against the earlier 268.05 and 268.06 second samples. Its frame SHA-256 was
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`,
matching the earlier sample. These individual runs do not establish a stable
speedup, full state equivalence, or native/RDP playability.
