# MyPC110 video-bank map correction

## Admission

T576 S4 is a corrective subtask of the immediately preceding task. It corrects
the App-owned map conclusion in S3 after stronger source evidence was found in
the already-approved Open-Source-PC110 research corpus.

## Objective

Restore the proven PC110 flash-window topology: bank 2 is the option video ROM
at CPU `E0000`, bank 3 is system firmware at `F0000`, and C000 receives the
bank-2 video image as its reset-visible, read-only shadow. EC/ED only controls
the existing later shadow write/relock transition. Retain a bounded diagnostic
to determine whether ordinary firmware initialization now reaches a different
finite boundary.

## Boundaries

- MyPC110 App firmware provider and its tests/diagnostic only.
- No Core, shared package, IVT/BDA, BIOS interception, QEMU injection or
  borrowed PC/AT ROM.
- No third-party source copy or raw asset commit.

## Evidence

Open-Source-PC110's documented live capture identifies the physical flash map:
E000 is bank 2 (`55 AA 47`) and F000 is bank 3. It explicitly identifies C000
as the shadowed destination. The preceding S3 C000 bank-2 alias therefore
contradicts stronger source evidence and must not remain.

## Verification

On both widths, App tests prove C000's reset shadow and E000 both expose bank
2, while F000 exposes bank 3. The bounded real-image
diagnostic must report its first successor fact without turning it into a boot
or display qualification. Complete repository-only unit suites, documentation
and whitespace gates remain required for S closure.
