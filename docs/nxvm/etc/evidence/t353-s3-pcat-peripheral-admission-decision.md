# T353 S3: PC/AT Peripheral Admission Decision

## Decision

T353 S3 does **not** admit a PC/AT port-`61h` PPI/speaker or another optional
peripheral. The repository holds no named DOS or Windows corpus that executes
or requires such a receiver, and it contains no selected hardware contract
that could define its 8253-channel-2, port, reset, deterministic-event, and
copied-host-audio boundaries. Adding a provider would therefore manufacture
device semantics rather than reconcile selected PC/AT topology.

The sole receiver remains
[PC/AT speaker/PPI `61h`](../../states/TODO.md).
Its existing admission condition is retained verbatim: a named DOS or Windows
corpus requiring gate/speaker timing; a documented 8253 channel-2 and
port-`61h` contract; deterministic event ownership; reset behavior; and a
copied host-audio boundary. Host audio time may not become guest time.

## Repository Corpus And Contract Inventory

| Surface searched | Result | Disposition |
| --- | --- | --- |
| Current DOS and Windows probes (`tests/machine/vm_dos_*`, `vm_windows31_*`) | The probes cover storage, video, keyboard/mouse, memory, timer/firmware, and Windows setup/checkpoint paths. None names, reads, writes, or asserts port `61h`, PPI, speaker, channel-2 gate, or speaker timing. | No qualifying corpus. |
| Committed media/fixture classification and source policy | The repository permits no bundled third-party guest media or firmware as a default dependency. No approved local image manifest names a PPI/speaker requirement. | No asset may be inferred or acquired for this S. |
| Default PC/AT profile and frozen core registration | T353 S2's exact 75-leaf contract contains no `61h` leaf. KBC owns only `60h` and `64h`; its former `60h--64h` envelope cannot imply `61h--63h`. `vm-pcat-topology-s2-smoke` proves `61h`, `62h`, and `63h` are not registered. | No selected port receiver exists. |
| PIT/RTC/NMI signal records | T350 retains PIT channel 0 to IRQ0 only. Its channel-2/PPI/speaker record and NMI decision both state that port `61h` must not be used to infer speaker, parity, I/O-channel-check, or NMI state. | Preserve separate owners and transfers. |
| Related optional interfaces | Serial, parallel, and game port have a separate `TODO(Low)` with a named-corpus and hardware-contract admission condition. PC/AT parity/I/O-channel NMI has a separate `TODO(Medium)` with source/latch/mask/lifecycle requirements. | Each already has exactly one receiver; none may be folded into S3. |

## Source And Topology Trace

The selected default PC/AT profile is the authoritative composition surface.
Its S2 leaf table names exact read/write ports and validates the descriptor
before core-machine publication. `core_machine_kbc_register_ports` registers
only `60h` and `64h`; the S2 topology smoke queries the frozen registry and
proves that `61h`, `62h`, and `63h` are absent. The core has no PPI/speaker
state owner, provider, reset/finalize path, deterministic due-event source, or
focused owner proof.

This is an absence classification, not a claim that PC/AT-compatible hardware
lacks the interface. It says only that NXVM's selected machine and its
repository-held corpus do not currently establish it.

## Transfer Integrity

| Gap | Sole receiver | Required future admission evidence |
| --- | --- | --- |
| Port `61h` PPI/speaker | `TODO(Medium)` PC/AT speaker/PPI `61h` | Named DOS/Windows corpus, documented channel-2/port contract, deterministic event and reset owner, copied host-audio boundary. |
| Parity/I/O-channel NMI source | `TODO(Medium)` PC/AT NMI source ownership | Selected board source, latch/mask/assert/deassert/reset contract, deterministic visibility, and delivery-consumer proof. |
| Serial, parallel, game interfaces | `TODO(Low)` serial, parallel, and game-port interfaces | One named corpus and hardware contract per selected controller, with core port/IRQ/DMA/reset/event ownership. |

Consequently S3 changes no runtime code, profile leaf, CMake target, test, or
artifact. S4 consumes this decision when it composes the selected topology
through reset, timeline, and firmware-visible paths.
