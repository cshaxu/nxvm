# T387 S3: Model 40 Bridge-Observation Contract

M5:T387:S3:MODEL40-BRIDGE-CONTRACT:OK

## Purpose And Decision Boundary

T387 S1 establishes the current deterministic callback order and T387 S2
separates the primary D4 oscillator facts from NXVM successful-retirement
ticks. The missing fact is not a ratio that may be inferred from a boot
symptom. This contract defines the only permitted future secondary bridge
experiment for the first receiver: the selected Model 40 system PIT
counter-1 refresh-pulse observation at port 61h.

The D4 source corpus identifies the system 8254 at 40h-43h, its compatible
1.19318 MHz source, and the Model-40-visible port-61h control/status paths.
The board-timing proposal names counter-1 refresh-pulse observability at that
port as the required CPU-to-PIT calibration receiver. The bridge may compare
the ordering and repeatability of this named observable. It cannot turn a
secondary implementation's scheduler, host elapsed time, internal counter, or
firmware instruction addresses into a Model-40 physical timing value or an
NXVM tick conversion.

## Admission Prerequisites

No capture may begin until the experiment record proves all of the following.

| Required identity | Required record | Rejection condition |
| --- | --- | --- |
| Reference machine | The reference explicitly supports the original 1986 Compaq DeskPro 386 Model 40 / 80386-16 system-board composition, rather than a generic AT, IBM 5170, clone, later DeskPro, or merely a CPU-compatible configuration. | No exact board identity or a substituted platform means the result is inadmissible and no trace is retained. |
| Reference build | Tool name, immutable source revision or vendor release hash, build options, host architecture, and an unmodified configuration file are recorded outside the product tree. | An unidentifiable or locally patched reference is inadmissible. |
| Firmware and media | Every ROM and guest image is owner-managed external input; its provenance, byte hash, and mounted/not-mounted state are recorded in the experiment note, never committed. | Missing identity, imported bytes, or a guest-dependent result outside the stated probe makes the run inadmissible. |
| Board selection | The configuration records 80386DX at 16 MHz, 1 MiB standard memory, the two 8254 blocks, dual 8237A topology, selected CECG and the Model-40 storage arrangement. | A selected-device or frequency substitution is not a Model-40 observation. |
| Observable route | The reference exposes a documented or inspectable route from system PIT channel 1 to the Model-40 port-61h refresh-pulse-visible state. | A port alias, a generic PC/AT refresh approximation, or an uninspectable route is not comparable. |

86Box, MAME and PCjs remain secondary candidates only. Existing IBM ibmat or
Model-339 configurations fail the first prerequisite and must not be relabelled
as Model 40. A candidate that fails qualification is useful only as a negative
disposition record; it cannot motivate a Core scalar or product change.

## Reproducible Input And Checkpoints

The future runner is an out-of-process research tool under
tools/research/differential/, never a product, test or release dependency. It
uses one deterministic, project-owned probe that does only the following:

1. cold-resets the qualified reference and records the named configuration
   identity;
2. programs the system PIT through 40h-43h with a fixed channel-1 control word
   and fixed count specified in the experiment record;
3. samples the documented port-61h refresh-pulse-visible state at explicit
   probe checkpoints; and
4. stops after the declared number of state transitions without booting DOS or
   deriving a result from POST progress.

The experiment record must retain these compact checkpoints, in this order:
configuration acceptance; PIT control/count writes; each masked port-61h
sample with probe sequence number; the first two state transitions if they
occur; and the terminal stop reason. It masks reference instruction addresses,
host timestamps, raw internal scheduler counters, unselected port-61h bits,
firmware-dependent register state, and all display/disk/IRQ activity not
caused by the probe. It compares state ordering and the declared probe-local
transition count only. It does not compare an unbounded instruction stream or
assert an expected host duration.

## Bounds, Cleanup And Result Classes

One invocation has a 30-second wall-clock limit, a 100,000 guest-instruction
limit when the reference can expose it, a five-second no-checkpoint limit, and
a 1 MiB raw-trace ceiling. The runner uses a unique ignored output directory,
retains a bounded final-event ring, and owns the complete child process tree.
On success, timeout, no progress, byte exhaustion or reference error it
terminates the owned tree, waits for trace handles to close, records only the
configuration identity, compact checkpoints, terminal reason and final byte
count, then deletes the raw trace. It removes experiment-specific reference
configuration and hooks from the active build after the verdict.

| Result | Permitted conclusion | Forbidden conclusion |
| --- | --- | --- |
| Qualified ordered transitions | The named PIT-to-port-61h route has a repeatable secondary ordering under the declared probe. | Any physical period, CPU-cycle count, NXVM retirement-tick ratio, DMA DCLK placement, or L3 claim. |
| No qualified reference or route | The calibration receiver remains unobserved; transfer it unchanged. | Filling the gap with IBM/AT, clone, source-code or boot-symptom inference. |
| Bound reached or divergent result | The particular experiment is inconclusive; preserve only the compact terminal record. | Treating a timeout or a matching boot symptom as a timing result. |

Any later proposal to publish a scalar must separately join a primary source
that defines both domains to a project-owned NXVM measurement and pass the
normal source, Core/VM ownership, regression and full-current-gate review.
This contract itself changes no code, timing behavior or L3 status.