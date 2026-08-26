# T484 S10 XT FDC Plan Implementation

`M5:T484:S10:XT-FDC-PLAN:OK`

`M5:T484:S10:XT-NO-AT-FDC-ALIAS:OK`

## Result

The selected IBM 5160-268 resolver now supplies one immutable drive-A media
slot and one Core FDC plan declaration:

```text
media ID 1: removable 40 x 2 x 9 x 512 when a session later supplies media
FDC:        3F2h write, 3F4h read, 3F5h read/write, IRQ6, DMA2
absent:     direction/control 3F7h route
```

The profile still has no runnable session because its independent ROM/B6
receiver remains unimplemented.  This S neither creates a default disk nor
changes firmware, host media persistence, or the product's existing session
admission boundary.

## Retained Owners And Minimal Mechanism

`core_machine_fdc` remains the only owner of guest controller registers,
command phase, reset, IRQ, DRQ and media access.  The profile supplies copied,
immutable route values; a later VM session can supply a media provider with
the selected geometry.  It does not store a second guest media or FDC state.

The shared Core FDC owner previously interpreted optional `direction_port` and
`control_port` value zero as an actual port-zero registration, including in
the pre-registration conflict scan.  It now registers and reserves either
route only when nonzero.  This is the one mechanism correction needed to
express a source-backed XT FDC without an AT `3F7h` alias.  Existing AT and
DeskPro configurations retain nonzero `3F7h` values and therefore retain their
existing route.

## Focused Proof

`vm-xt-5160-268-profile-smoke` creates a project-owned in-memory removable
media provider with `720` logical sectors, `512` bytes per sector, `40`
cylinders, `2` heads and `9` sectors per track.  It proves the resolver's
frozen drive slot/configuration, creates a Core plan, binds that one provider,
and verifies:

- Core exposes FDC reads at `3F4h`/`3F5h` and writes at `3F2h`/`3F5h`.
- The FDC DMA capability is the Core-issued channel-2 binding and IRQ is 6.
- Neither read nor write provider exists at `3F7h` for this XT plan.
- The existing unsupported XT session and fixed-request checks continue to
  prove this is not a hidden PC/AT runtime alias.

The direct smoke emitted both S10 markers.  The full Debug
`current-gate` passed.  The stripped Release target `vm-0-5-0484` passed its
optimized-release check and produced
`build/output/nxvm_0_5_0484.exe` with SHA-256
`73D252A9C79D74725898207D43D3DCDEC01D2A1CDA7A527F2591564E5E0E25CA`.
Documentation governance also passed.

## Simplicity Accounting And Transfer

Tracked production code changed by `+21/-8` lines across the Core FDC owner,
its board-registration check and the XT resolver/header; the focused existing
profile smoke changed by `+55/-4` lines.  No new source target, controller,
media cache, session route, or compatibility wrapper was introduced.  The
retained production path is `profile -> copied Core plan -> Core FDC -> one
media provider`.

The selected logical functional route is complete for B3.  Spindle, seek,
head-load, index and board service duration remain absent from the selected
mechanism source; they stay transferred to the following XT board/device
phase-timing task and are not published as an FDC deadline or host-pacing
value here.
