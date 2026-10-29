# T582 S5: CPU timing computation audit

A task-owned direct-build instrumentation of the current CPU timing model ran
the existing private PC110 x64 Release probe for exactly 10,000,000 retired
instructions. It recorded 62,401,672 prefix-count invocations and 87,736,652
prefix bytes visited; the guest checkpoint was 79,964,294 ticks and PC
0003dbc5. Instrumentation adds overhead and the measured 22.208 seconds is
not a speedup estimate. This is an early startup sample, not a whole-product
or interactive claim. CMOS/media were copied; original inputs were untouched.

Source inspection finds 28 call sites across CPU source, including the 486
source-cost path and its nested nominal, memory, decode and bus calculations.
The existing function depends on profile, opcode length, bytes, and V30's
8080-emulation mode; replacing it with a static global or a persistent mirrored
state is therefore not an acceptable optimization. The next step is to map
which helpers share immutable decoded data within one 486 calculation and
prototype passing one local count to those helpers. Standalone callers and
other profiles retain their existing entry paths. No production changes are
made until a same-compiler Release fixed-work pair proves an actual gain and
CPU timing, prefix, V30, preview/runtime tests retain equivalent results.

## Helper ownership and regression constraint

The nominal, L2, memory and decode helpers are all invoked from the single
80486 source-cost production path. Their declarations are in the private CPU
timing header, but the existing cpu_486_nominal_timing_smoke also invokes them
directly for source-row and invalid-argument coverage. A refactor must retain
those checks without adding duplicate production entry paths: either move
those focused checks to the complete source-cost owner or adjust the existing
fixture to supply explicit decoded facts. Memory/decode/nominal helpers each
independently reject faulted/invalid context and use the same decoded prefix
count; these checks cannot be dropped without proving the enclosing path owns
them. The bus helper lives in cpu_bus.c and needs a separate call-graph audit.
No signature or production code has been changed.

## Preliminary private candidate

A task-owned prototype passes the already computed prefix count to the four
486 nominal/L2/memory/decode helpers. It is deliberately not a final design:
the temporary build removes standalone test entry points, so it only measures
the complete production path. Both variants are directly compiled with the
same Release flags and linked to the same cached libraries. At 10 million
retired instructions and identical 79,964,294 ticks / PC 0003dbc5, x64 ran
19,812 versus 19,406 ms and x86 26,013 versus 25,326 ms. One pair is not
sufficient to distinguish a small gain from host variability; a longer 30M
fixed-work pair is running. The standalone timing tests and invalid argument
contracts still need a single-owner final design if the result warrants it.

## 30-million-instruction comparison

With identical Release flags and private initial inputs, x64 baseline ran
59,148 ms and candidate 56,109 ms; x86 baseline ran 70,851 ms and candidate
72,000 ms. All four retired 30,000,000 instructions at 195,624,840 ticks and
PC 000f6440. This single ordered pair per width is not a statistical study.
The observed x86 direction conflicts with the earlier 10M sample; neither a
cross-architecture improvement nor a stable regression is established. Do not
land the prototype based on these results. Investigate profiling and dataflow
before choosing whether a single-path implementation is worthwhile.

## Display observation ownership follow-up

The late-guest S1 sample identified F65535 capture as a larger candidate
than prefix counting. Current Machine display publication observes the
standard Board VADP generation, while PC110 binds an F65535 snapshot provider.
The F65535 internally tracks generation and captured_generation but the
existing provider slot accepts only a capture callback. Therefore the standard
VADP observation cannot establish whether a custom F65535 frame is unchanged;
full capture can precede the existing changed-flag rejection. This is a
source-path observation, not yet a measured current cost or proof that the
VADP observation incorrectly suppresses a PC110 frame. A next audit should
inspect board configuration and observation return values and trace actual
PC110 display calls before proposing a single-owner observation contract.
It must include cursor phase, force capture, generation wrap and failed capture,
and preserve full snapshot semantics. No additional state or interface is
proposed until the existing contracts and actual costs are established.

### Measured early display publication

A 30-second x64 public-factory PC110 probe, with private CMOS/media and
existing Emulator Machine publishing, wrapped the two existing observation
and capture calls. After worker shutdown it reported 1,585 successful Board
observations, zero observation skips, and 1,542 F65535 captures: 45 changed,
1,456 unchanged and 41 unsupported/failed captures. Thus 94.4% of attempted
captures in this early sample returned a complete unchanged snapshot that
Machine subsequently discarded. This confirms actual repeated work, not its
CPU-time percentage or behavior in later gameplay; wrapper overhead and frame
file output affect throughput. The probe exited zero. It does not prove a
standard VADP observation can incorrectly suppress PC110 frames.

The existing F65535 capture owns generation and prior cursor visibility;
cursor visibility depends on VSync and XR60, independently of pixel changes.
An admission check must account for both and retain forced capture and failure
semantics. The current board observer is tied to the standard VADP, while the
custom provider slot only has capture; consequently any generic observation
extension needs a separately designed existing-owner contract. Do not return
an incomplete snapshot from the existing capture function or omit blink frames.

### Proposed single-owner design for a following S

Extend the existing display-provider binding with an optional observation
callback returning the existing x86_video_snapshot_observation. Machine asks
the currently bound provider, never a different VADP; no observer means
conservative capture. Standard VADP retains its existing observation. PC110
passes its already-owned guest VSync count into an F65535 observation that
checks generation and conservatively requests capture on cursor phase change.
Observation is read-only; only successful capture/publish may acknowledge a
generation. Force always captures; unsupported, invalid and failed calls keep
the old output and propagate through the current status contract. Handle reset
and wrap conservatively, and do not let skip turn a previously failed first
capture into success. This design needs an existing binding/construction
contract extension in X86-NXVM and App wiring, not Lib or Emulator changes.
It is an estimated +75/-15 production and +70/-0 existing-test change, not an
implemented or measured result. A following S must prototype and compare
actual guest workload and verify cursor, force, reset, failure and frame
equivalence before any production implementation.
