# T546 S19 CPU External/NPX/Bus Evidence

## Scope and sources

This packet closes only the CPU/Core boundary for `WAIT`, external `TEST`,
NPX busy completion and the affected ESC paths.  It does not implement x87
arithmetic, add a host wait, add a BIOS shortcut, or create a second executor.

Intel 8086/8088 and 80186 manuals in the approved external manual archive
define `WAIT` as an interruptible test-pin wait: 8086/8088 use `3 + 5n`
clocks and 80186 uses `6 + 5n`.  Those rows remain Manual-L3.  The 80286
manual defines successful `WAIT` timing, but does not supply a complete
controller-service model for an attached coprocessor; the restart/poll
arbitration therefore remains the explicitly marked L2 control model.  The
80386 external wait/service route likewise remains L2 rather than inventing a
physical NPX service rate.  External emulator sources were used only as
read-only L2 cross-checks; no external implementation was imported.

## Implemented ownership

- CPU owns the instruction state: `WAIT`/automatic ESC restart is a stall, not
  a retired synthetic instruction.
- Core owns arbitration and guest time.  Board TEST and escape-trap inputs are
  copied through the existing attachment/CPU-bus boundary; absent board TEST
  is derived from the existing FPU busy deadline.
- The scheduler advances device time, FPU completion and CPU wait polling on
  the same guest-time path.  No host sleep or VM timing path was added.
- A delivered NMI/IRQ clears the interruptible `WAIT` state.  80186 escape
  trap input delivers the existing #NM route.  80286 automatic ESC restart
  re-executes only after the FPU deadline releases TEST.

## Direct proof

`machine_fpu_interface_s65_smoke` covers 8086/80186 L3 startup and poll
formulas, Core-scheduler FPU completion, 80286 busy ESC restart, accepted IRQ
wake from `WAIT`, and 80186 #NM escape delivery.  Existing FPU contract,
timing-ledger, retirement and LOCK owner tests remain receivers.

After the final test/registration corrections:

- x64 repository units: 506/506, including the negative CPU-bus boundary;
- x86 repository units: 506/506, including the negative CPU-bus boundary;
- both-width current specialized gates: passed;
- no new L1 disposition was introduced in this packet.

## Receiving artifacts

The current 0546 optimized artifacts are present as the four profile pairs:

- `my5160`: x64 `3AF6DCCAA6546CF50B3CC205FC7F51CC0DCFFC9360253A7862C9317B5A01B176`,
  x86 `D9062FD885113325599E1BE64A46D4BE396866522545808F95FBCE384BD1CC42`;
- `my5170`: x64 `8EAF1B656DA340FFA2DCB1A74A87958CEBDFF1FEED85317CB7310EBB8D707D5F`,
  x86 `AC9E86FDA1099E6A9C76056FC58905A92A4E8F29ED460AB1F47C52E2C876C0B8`;
- `mydeskpro386`: x64 `0C8E2EF054E1B66237B071E30AEC2C2852D5C29AAE886964AD3F3FF91A209B97`,
  x86 `2932AF767764CB813DFF2973A79B3FB9247BC57077CC39B25AD404A3CB3BA9A4`;
- `nxvm`: x64 `A8F6295D0780C4D833DF9BE5686C40EB9ACEE055C3D71AB4279CB61AE698BE5E`,
  x86 `1C35CFABC5B8FC90B4F69E0EFD1CBD8FE756E408418E2160C907C4B3D84FE6CC`.

PE headers confirm each stated x64/x86 architecture. MyNES inputs, INIs,
media and snapshots are outside this packet.
