# T569 S1 — PC110 Power And APM Source Gate

## Scope and method

This audit compares current MyPC110, VL82C420 and Core ownership against the
current read-only Open-Source-PC110, PC110-EMU and PC110-QEMU trees.  It adds
no firmware byte, third-party code, protected media or production route.

## Findings

| Candidate | Evidence | Current owner | Disposition |
| --- | --- | --- | --- |
| `INT 15h AX=5300h/530Ah` APM power status | Open-Source-PC110 `Software/PS2TUI/README.md` uses the BIOS service; `Discovery/PSU-MB-M38/readme.md` identifies it as the cooked telemetry route. | The selected PC110 BIOS, not MyPC110, Lib or Emulator. | Do not intercept or synthesize it. It can only be qualified when the real firmware executes in the later boot receiver. |
| `0ECh/0EDh` | Open-Source-PC110 `Discovery/Chipset/readme.md` §13j.5 and the correction banner in `Discovery/PSU-MB-M38/readme.md` identify the pair as VL82C420 shadow/cache/ROM configuration. | Existing App route to `x86_vl82c420`. | Already correct; no power-MCU alias may be installed. |
| Block2 `FFh=82h`, then `HLT` | Open-Source-PC110 `Discovery/Chipset/readme.md` records the observed suspend sequence; `B8h` bit 3 selects the resume branch. | Existing VL82C420 register bank stores the accesses. | The register fact is L3, but complete suspend is not admitted: no source-qualified wake producer, resume image/flag transaction, reset relation or guest-visible failure contract is present. |
| U6/M38223 physical sequencing | Open-Source-PC110 `Discovery/PSU-MB-M38/readme.md` establishes rails, MCU and serial telemetry. | No x86-visible endpoint exists in current source. | Physical wiring is not a CPU protocol; do not emulate the MCU or alias it to Emulator lifecycle. |
| PC110-EMU/QEMU indexed power windows | Their implementations expose `0ECh/0EDh` arrays. | External reference only. | Reject for production: the current Open-Source-PC110 correction proves that decode belongs to VL82C420. |

## Existing mechanism review

`x86_vl82c420` already provides the sole indexed storage for Block2, SCAMP and
EC/ED.  `app-mypc110/pc110_config.c` is the single port-route owner.  Core
offers stop-clock, SMI, NMI and reset inputs, but none supplies a PC110 wake
source or permits a board to claim that a host pause/resume is a guest power
event.  Wiring Block2 `FFh` directly to `set_stop_clock()` would therefore
leave the guest without a qualified wake/error protocol and create a second
lifecycle meaning.

No production or test change is warranted: a new register test would only
duplicate existing indexed-bank coverage, while a fake APM or suspend test
would validate behavior that has no lawful source contract.

## Transfer

The real-firmware boot receiver must execute the selected BIOS before it can
qualify `INT 15h` services or the observed suspend sequence.  It must then
trace the exact Block2 writes, `HLT`, resume flag, preserving state and wake
source.  Only a complete trace can admit an App-local power receiver.  The
storage receiver remains independently ordered and is not expanded by this
audit.
