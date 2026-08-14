# M5 PC/AT 8042 And AUX Completeness

Complete the selected PC/AT 8042 controller, keyboard, and AUX signal path as
one deterministic state-machine package. Every retained command, response,
FIFO byte, IRQ1/IRQ12 transition, A20/reset side effect, timeout, and reset or
finalize path must have one controller owner, one manual requirement, and
focused proof. The goal is reliable guest-visible controller behavior for the
selected PC/AT profile, not a generic host-input bridge or a claim of every
PS/2 clone feature.

## Planned subtasks

| Subtask | Objective | Boundary |
| --- | --- | --- |
| S1 | Build an Intel 8042, IBM PC/AT keyboard/AUX manual-to-source-to-proof ledger. Classify controller commands/status bits, command-byte/output-port state, keyboard protocol, AUX protocol, IRQ routes, deterministic scheduling, profile wiring, reset/finalize, and every existing proof. | Audit only. No source import and no behavior claim. |
| S2 | Reconcile the controller-owned 8042 command/status, input/output FIFO, command byte, output port A20/reset, self/interface tests, IRQ1/IRQ12 gating, and reset/finalize lifecycle. | No keyboard or AUX device protocol expansion beyond controller routing. |
| S3 | Reconcile the selected AT keyboard command, ACK/RESEND/BAT/identify, LED/typematic/scan-set, queued scan-code, response-delay, IRQ1, and reset behavior. | No host keyboard passthrough, firmware rewrite, or guest-memory shortcut. |
| S4 | Reconcile the selected PS/2 AUX command/status/report protocol, response delay, IRQ12, packet framing, and reset behavior needed by the chosen corpus. | Wheel identification, remote/read-data, scaling, and resend require a manual/corpus decision; transfer any unselected feature precisely. |
| S5 | Audit the combined controller/keyboard/AUX lifecycle, equal-tick ordering, profile/port composition, no-publication failures, and all explicit transfers before task closure. | No port-61/PPI, arbitrary PS/2 clone extensions, generic host-input API, or Windows claim. |

## Required method and exit standard

Use the Intel 8042 contract and IBM PC/AT technical reference as primary
behavior sources. PS/2 device documentation or project-owned deterministic
probes may select additional AUX commands only when they name a real consumer.
Bochs and PCjs may guide questions but their source is never imported or used as
the specification. Sweep all controller port registrations, host-event mapping,
timeline advance, delayed responses, FIFO publication, IRQ assert/deassert,
reset/finalize, A20/reset callbacks, profile declarations, and focused tests.

The task can transfer an unselected keyboard or AUX behavior only to a named
Queue/TODO receiver with its manual/corpus admission condition. It cannot hide
an absent command behind a generic "PS/2" label, make a test-only pending byte
public, or bypass the one controller-owned FIFO. Task closure requires a stable
deterministic-L3 state graph for every selected controller signal and all
relevant current-gate, governance, and source-boundary proof.
