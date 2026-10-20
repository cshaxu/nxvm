# M1 T4 S14 — control fallback attribution

## Scope

S13 advances the selected RAIDEN II set through the bounded Video CRTC write
extension and leaves the first independent CPU fault at `0x658`. Neither the
current MAME map nor the independently inspected FBNeo route establishes a
lawful receiver contract for that address. This S therefore does not model or
accept it.

## Model

The existing Board fallback already refuses unowned byte accesses with
`LIB_STATUS_INVALID_STATE`. It now records the first such access after reset as
four neutral facts: validity, physical address, byte width and Core read/write
direction. It does not retain source/destination bytes, guest program counter,
registers or a trace. Further failures do not overwrite the first fact; reset
clears it.

This is runtime attribution only, not an L2 receiver model or a physical
hardware claim.

## Required verification

- Board unit coverage uses real Core memory routes to prove first-access
  retention, unchanged failure, and reset clearing.
- The owner-supplied selected-set probe reports the bounded observation on
  both host widths without retaining protected payload.
- Complete repository-only unit suites, documentation governance and whitespace
  checks run after the production change.

## Observed result

Both x64 and x86 selected-set probes preserve the existing CPU fault at decimal
`1624` (`0x658`) and report the same bounded observation:

```text
control-valid=1 control-physical=1624 control-bytes=1 control-access=1
```

`control-access=1` is the existing Core write enum. The Board still returns
`LIB_STATUS_INVALID_STATE`; this result establishes only that the first missing
contract is a byte write.

## Stop boundary

The observation cannot justify accepting `0x658`, widening the fallback or
adding generic Board RAM. A later receiver requires independent contract
evidence or an explicitly approved L2 model.
