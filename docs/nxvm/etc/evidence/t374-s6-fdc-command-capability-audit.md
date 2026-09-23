# T374 S6: Model-339 FDC Command-Capability Audit

## Authority And Classification Method

The selected controller surface is `CORE_MACHINE_DEVICE_FDC`, labelled Intel
8272A. The [Intel 8272 command table](https://cpctech.cpcwiki.de/docs/i8272/8272sp.htm)
defines exactly fifteen commands and defines every other opcode as a one-byte,
non-interrupting invalid-command result of ST0 `80h`. It is used here as a
controller-semantic source only; physical transfer intervals, polling cadence,
drive signals and board timing remain unconverted. The NEC uPD765 source
already admitted by T372/S4 is the compatible controller cross-reference.

The audit inspected every case in `core_machine_fdc_command_length()` and
`core_machine_fdc_execute()`, all command modifier handling, current media
provider operations/capabilities, FDC topology configuration and all focused
FDC tests. "Complete" below means only that an implementation route exists; it
does not assert full command-parameter, media-layout or timing conformance.

| 8272A command | Current route | Functional disposition / receiver |
| --- | --- | --- |
| Specify `03h`; Sense Drive Status `04h`; Recalibrate `07h`; Sense Interrupt `08h`; Seek `0Fh` | Explicit command-length and execute cases. | Present owner route; parameter/state completeness remains T374 audit work. |
| Read Data `06h`; Write Data `05h`; Read Track `02h`; Read ID `0Ah`; Format Track `0Dh` | Explicit transfer/result cases. | Present nominal route, but fixed 512-byte media layout, partial modifier handling and synthetic track/ID representation require later functional receivers. |
| Read Deleted `0Ch`; Write Deleted `09h` | No case; current default returns invalid. | Missing selected-controller command. Requires a media address-mark/deleted-data metadata contract before FDC implementation; do not emulate it with ordinary read/write. |
| Scan Equal `11h`; Scan Low or Equal `19h`; Scan High or Equal `1Dh` | No case; current default returns invalid. | Missing selected-controller command. Requires FDC-side compare semantics plus a bounded DMA/non-DMA input path; it cannot be represented by ordinary sector I/O alone. |
| Multi-track, skip, `N`/`DTL`, `EOT`, `GPL` and scan-step parameters | Current transfer accepts only `N=2`, maps a linear 512-byte geometry and ignores or restricts several source-defined command modifiers. | Existing nominal commands are incomplete. Split this from deleted/scan so every chosen repair has one media/transfer owner and no hidden mechanics timing. |
| `VERSION 10h` | Explicit current case returns `90h`. | **Over-admitted:** `10h` is absent from the 8272A command table and must therefore return the already-owned invalid `80h` result for Model 339. No caller/test depends on `90h`; select the bounded removal as next repair. |
| All other opcode patterns | Default returns one-byte `80h` result and no IRQ. | Source-compatible invalid route, subject to the selected `VERSION` correction. |

## Selected Next Repair

T374 S7 may remove the `VERSION` command case from the command-length and
execute dispatch so `10h` reaches the existing invalid-command result. Its
cohesive FDC smoke must prove `10h` yields only ST0 `80h`, produces no IRQ and
returns to command phase. This does not add an enhanced FDC capability,
change an FDC public interface, or erase the separately recorded command
gaps.

## Similar-Issue Sweep And Transfers

The only command implemented by current dispatcher but absent from the 8272A
table is `VERSION`. No in-repository caller, profile descriptor or focused
test requests `90h`. The three Deleted/Scan families are not "invalid by
policy": they are documented controller commands and remain explicit T374
functional work behind their required media/compare contracts. Fixed `N=2`
and modifier omissions are likewise open functional work, not timing claims.

Focused Git-Bash CMake replay confirms the unchanged baseline:

```text
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T290:S1:FDC:PORT:OK
M5:T291:S1:FDC:PORT:OK
M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK
```

No ROM, firmware, guest media, source import, local asset, timing scalar or
Model-339 L3 claim is introduced.
