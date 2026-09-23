# M4 First Supported Games Plan

## Support Boundary

M4 supports exactly two owner-local, nonredistributed NTSC iNES inputs. They
remain ignored under `assets/`; their names and possession establish neither
redistribution rights nor a repository dependency.

| Support input | Header observation | Required profile | M4 role |
| --- | --- | --- | --- |
| Dr. Mario | 32 KiB PRG-ROM, 32 KiB CHR-ROM, mapper 1, no trainer/battery | MMC1, standard controller, RP2A03 audio | First complete-game path. |
| Super Mario Bros. 2 | 128 KiB PRG-ROM, 128 KiB CHR-ROM, mapper 4, vertical initial mirroring, no trainer/battery; local image carries the legacy 128-byte display-title suffix | MMC3 including qualified PPU-A12 IRQ, standard controller, RP2A03 audio | Second complete-game path and raster/IRQ qualification. |

The observations are local iNES header facts, not board provenance or a claim
that every dump with the same title behaves identically. Tests committed to this
repository remain project-authored mapper/APU fixtures. Owner-local game runs
have a bounded manual protocol and may record only stated observations, not ROM
bytes, machine paths or unbounded traces.

The removable-media boundary accepts only the historically documented 127- or
128-byte iNES display-title suffix after an otherwise exact payload, then passes
the exact logical payload to Cartridge. It rejects every other surplus length;
this is container normalization, never mapper data.

## Complete Experience Contract

Each support input must pass the following on x64 and x86 in both configured
KVM representations:

1. Insert from `mynes.ini` or `rom insert`, enter the cooked monitor, issue
   `start`, and reach its controllable title/gameplay state without a Core trap.
2. Use the default P1 mapping to make visible game-state changes: D-pad motion,
   J/B, K/A and Enter/Start where the title/game state accepts them.
3. Hear continuous 48 kHz mono RP2A03 audio through pause/resume boundaries;
   pausing or returning to monitor flushes pending sound, and resuming does not
   replay a stale tail.
4. Maintain one-times guest speed through a ten-minute owner-local play run:
   no unbounded host CPU spin, frame backlog or audio queue growth; a host stall
   may report an underrun but cannot advance dropped guest cycles.
5. Exercise Esc, cooked `pause`/`resume`, `rom eject`, reinsertion and `exit`.
   No held control, sound buffer, native surface or reader survives a transition.

Window proof observes pixels as in M3; Console proof observes Core-produced
80x25 text cells. The Console is never an alternate hardware renderer. A title
screen alone is insufficient; at least one controllable in-game transition per
support input is required.

## Hardware And Integration Scope

M4 adds no general compatibility promise. It owns only:

- strict iNES mapper-1/MMC1 and mapper-4/MMC3 profiles needed by the named
  inputs, including cartridge-controlled nametable mapping and ROM bank bounds;
- MMC1 serial register behavior, PRG/CHR banking and reset behavior;
- MMC3 bank registers, PRG/CHR mode, mirroring and PPU A12-qualified IRQ
  counter/line behavior needed by the support flow;
- RP2A03's two pulse, triangle, noise and DMC channels, frame sequencer,
  status/IRQ and DMC arbitration through the existing machine clock;
- a neutral copied PCM stream, Core sample conversion and a single
  stop/wake-aware 1x host-pacing path.

Core owns deterministic mapper/APU/guest time. App composition owns neutral
audio-device lifetime. Common continues to own execution/lifecycle; Lib owns
host audio. MyNes supplies the generic copied PCM component in its own Lib under
the owner-approved M4 scope. SoftPC is not changed; a later shared-corpus
recovery may take the self-contained interface back upstream.

## Ordered Implementation Candidates

| Candidate | Depends on | Concrete outcome |
| --- | --- | --- |
| Mapper-1 Dr. Mario profile | M3 | A bounded MMC1 profile accepts the first support input; project fixtures prove serial writes, banking and mirroring. Actual game execution waits for APU support. |
| Mapper-4 Super Mario Bros. 2 profile | Mapper-1 profile's cartridge abstraction | A bounded MMC3 profile accepts the second input; project fixtures prove bank selection and qualified IRQ behavior. Actual game execution waits for APU support. |
| RP2A03 audio hardware | M3 | Deterministic five-channel APU, frame IRQ and DMC arbitration produce bounded PCM blocks from the single machine clock. |
| Neutral audio adoption and pacing | APU hardware | MyNes Lib copied PCM lifecycle, active/flush semantics and 1x host pacing make sound audible without changing guest time. |
| Supported-game acceptance | All prior candidates | Both owner-local inputs complete the stated Window/Console/audio/speed/management protocol on x64 and x86. |

The first three candidates may progress independently once their shared
cartridge/APU ownership surfaces are explicitly selected. The fourth supplies
the MyNes Lib host-audio prerequisite and must retain a neutral public contract.
The final candidate repairs an observed
support-flow defect if necessary; it is not an audit-only substitute for prior
outcomes.

## Evidence Rules

Each mapper/APU candidate supplies project-owned unit and integration fixtures
that use production CPU/MMIO paths. Local game runs record ROM role, selected
video mode, scenario steps, observable transitions, duration, architecture and
cleanup outcome. They do not assert visual/audio hashes from protected bytes.
Audio proof includes sample count/continuity and bounded queue behavior; human
audibility is a supplemental observation. All acceptance runs include both
Window and text-only Console routes.

M4 excludes FDS, PAL/Dendy, expansion audio, battery persistence, save states,
additional controllers, other mappers and all claims about arbitrary ROM dumps.
