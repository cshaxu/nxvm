# T456 S5: PIC Default-IR7 Acknowledgement

## Defect and source boundary

T456 S4 incorrectly claimed complete `PIC-F8` proof. The rendered Intel 8259A
`231468-003` result retained by T456 S1 says an acknowledgement that finds no
request produces default level 7 behavior. Before this S,
`core_machine_pic_get_interrupt` returned zero on every no-selection path;
that contradicted the initialized 8086/88 PIC vector contract.

This is an L3 logical result only. The owner reconfirmed on 2026-08-24 that
L4 is not required here. It does not attempt to decide when an external
request electrically disappears, reproduce a slave spurious race, or model
INTA waveforms or elapsed time.

## One-owner correction

The existing `get` acknowledgement owner now returns `master.ICW2 | 7` when
the initialized master has no selected request. It leaves IRR and ISR intact.
Before ICW initialization completes, it retains zero because there is no
programmed vector base. `scan` and non-mutating `peek` retain their existing
no-request result, so CPU keeps S2's one `scan`-gated delivery path and cannot
invent a spurious interrupt by ordinary execution.

## Verification

The existing PIC command-priority smoke proves an initialized empty pair has:

- `scan == false` and `peek == 0`;
- `get == 0x0f` for master ICW2 `0x08`; and
- unchanged IRR and ISR after that acknowledgement.

Its ordinary IRQ and cascade vectors remain covered in the same fixture. The
PIC lifecycle smoke now explicitly expects the same `0x0f` acknowledgement
after initialized invalid source binds. The PIC phase-S2 and CPU
hardware-delivery smokes prove the regular scan-gated CPU route remains
unchanged.

- Rebuilt and ran the two directly affected PIC smokes: 2/2 passed.
- The configured build and standalone documentation governance check passed.
- The isolated full current-gate replay passed 293/293 in 97.54 seconds.

The production change is a five-line no-selection result in the existing PIC
acknowledgement owner; no state, wrapper, interface or delivery path is added.
