# T507 S14 L1/L2 Deadline Correction

`M5:T507:S14:ACTIVE`

## Source And Reference Boundary

Intel 8237A order `231466-005` and Intel 8272A order `210608-001` are archived
under `../../../../nxvm-assets/manuals/fdc`.  The latter states the FDC's
15-us MFM and 31-us FM service bounds.  Visual source review is primary; OCR
was used only to locate pages.  Read-only comparison covered local 86Box,
Bochs and PCjs sources.  86Box uses controller-owned timers (IDE base 10 us,
MFM command waits derived from a 10-us base); Bochs also has controller timers;
PCjs confirms owner/wiring shape but labels several disk delays arbitrary.  No
third-party code or timing literal was imported.  MAME was not locally present.

## Finite Correction Matrix

| Owner | Prior defect | Correct Core route | Level |
| --- | --- | --- | --- |
| DMA | The scheduler classified every pending DMA request as L1, even when a frozen DMA clock ratio existed. | Any nonzero copied DMA ratio publishes the next one-unit deadline; only a genuinely ratio-less request remains L1. | Source L3 where its existing timing rule qualifies it; otherwise explicit L2 conversion. |
| FDC | A whole-number ticks-per-microsecond field could not represent the XT 4,772,727-Hz macro axis and forced an avoidable no-delay fallback. | The existing FDC owner converts manual microseconds from one copied ticks-per-second ratio using an integer ceiling. | Manual L3 formula with a qualified board ratio; L2 with a macro ratio. |
| ATA / WD1003 / Compaq-WD | Task-file phases were one immediate scheduler step, making every active service phase L1. | HDC owns elapsed time and one pending service deadline.  The unqualified default ATA axis publishes an immediate owner deadline without inventing a duration; WD1003 and Compaq/WD use 16,000 ticks (2 ms) on their 8-MHz macro axes. | Other-L2: reference-derived macro service quantum where a macro axis exists; otherwise an explicit immediate owner boundary, not a mechanical or physical-timing claim. |
| XT Xebec | Its observable transfer is DMA3-driven, not an unscheduled task-file phase. | Existing DMA request/transaction route supplies the deadline; no unused Xebec service field is added. | Existing DMA L2/L3 disposition. |
| Model-40 D4 | A single refresh hold was categorised as a duration-less L1 blocker. | The existing D4 owner publishes one next-Core-step ordered transaction. | L2 ordered quantum; no fictitious duration. |

No active production profile retains a DMA L1 classification: default PC/AT,
Model-339, Model-40 and XT each freeze a nonzero DMA ratio.  A deliberately
unqualified test/configuration still uses the existing bounded L1 escape; this
is the only remaining L1 class and is not silently promoted.

## Architecture And Minimality

Core remains the sole writer of elapsed guest time.  Profile construction only
copies immutable ratios/quanta; VM neither calculates ticks nor observes device
internals.  FDC and HDC publish their own earliest due point; the scheduler only
chooses the minimum and advances the same owner.  The direct HDC helper remains
an owner-local immediate-service test/client convenience, while production uses
the elapsed-tick entry point.  No controller, queue, media cache, host timer,
profile callback or parallel completion path was introduced.

## Required Proof

- Focused scheduler/DMA/D4, FDC/HDC/profile, and restored HDD/Windows probes pass.
- Complete repository-only unit suite: 312/312 passed.
- External-asset integration suite: 20/20 passed.
- Documentation governance passed.  Rebuilt stripped Release
  `nxvm_0_5_0507.exe` is 1,242,996 bytes; SHA-256
  `DB2A9B7F23A3EC88E91F873CAC4C4B1299BF8BC5FDDE0C0DB8B716C6B1C433F9`.
