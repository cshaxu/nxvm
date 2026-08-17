# T390 S29: Model-40 ROM Decode Correction

Marker: M5:T390:S29:MODEL40-ROM-DECODE:OK

## Evidence and correction

The owner-directed review compared the Model-40 external-ROM profile with the
public PCjs DeskPro 386 Rev-E configuration and its public reset-mapping
explanation. The neutral result is a 32 KiB interleaved system-ROM image at
the top of the first MiB, a lower F-segment compatibility alias, and two
80386 high-address aliases needed by the reset addressing model. The prior
E-segment 128 KiB repeated window was not supported by that evidence.

The profile now materializes one 32 KiB interleaved image and registers its
four explicit decode windows through the existing immutable-ROM and alias
contracts. The E segment is no longer invented as ROM and remains available to
the existing Model-40 D4 replacement owner. The required four ROM routes plus
the existing machine devices exceed the former private fixed route capacity, so
the core-private capacity is raised to twelve. This changes no route, firmware
provider, address decode, or ROM content for any other profile.

## Owned proof

The existing Model-40 D4-map smoke uses only project-owned synthetic chips. It
proves both low F-segment windows and both high reset windows, enables A20
before asserting the high aliases, proves the E segment is writable before D4
replacement takes ownership, and retains the D4 protection/reset assertions.
The Model-40 BYOB smoke continues to pass. No vendor ROM, media, identity,
path, digest, byte, trace, PC, or provenance is retained here.

## Boundary

This is a board-local address-decode correction only. It does not validate
POST, boot media, chipset timing, CPU retirement timing, board clocks, or L3.
The paused C0-to-C1 capture remains a separate later receiver.

## Research references

- [PCjs Rev-E DeskPro 386 EGA configuration](https://www.pcjs.org/machines/pcx86/compaq/deskpro386/ega/2048kb/machine.xml)
- [PCjs DeskPro 386 reset-mapping explanation](https://www.pcjs.org/blog/2015/04/16/)
- [PCjs DeskPro 386 ROM catalog](https://www.pcjs.org/machines/pcx86/compaq/deskpro386/rom/)