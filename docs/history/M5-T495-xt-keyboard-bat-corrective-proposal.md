# Withdrawn: M5 T495 IBM 5160 Keyboard BAT Corrective

## Admission Boundary

The external DOS boot probe found that the selected IBM 5160 firmware reaches
its keyboard reset wait with no Core-owned keyboard BAT producer.  The earlier
PPI audit verified the port and IRQ route, but its BAT smoke injected `AA`
through the host-input ingress and therefore did not prove the keyboard reset
device.

This corrective scope was withdrawn before acceptance.  The completed 8255/PPI
owner may retain its ports, latch and IRQ publication, but it must not own a
keyboard's serial transport, reset, BAT or keyboard-side buffer.  Those are a
separate device boundary, now admitted as T496.  Firmware, VM, tests and host
input may not publish a second BAT path.

## Evidence And Timing Disposition

The initial repository corpus lacked the required 1986 keyboard chapter. T496
has now acquired and visually verified the original IBM March 1986 reference;
it establishes the reset/BAT protocol. This proposal remains withdrawn because
the defect was the PPI owner, not the absence of a keyboard behavior. The
current 8088 Core elapsed axis is not a verified physical retirement axis, so a
deterministic choice inside a manual range must still remain macro/L2.

External emulator inspection is corroboration only.  It may confirm the
reset/BAT state order, but may neither override IBM material nor introduce
source text or code into this repository.

## Completion Standard

No implementation or validation result belongs to this withdrawn proposal.
T496's source/List-1/List-2 sequence determines the one correct device owner
and any source-supported reset behavior before a later implementation batch or
BYOB rerun is admitted.
