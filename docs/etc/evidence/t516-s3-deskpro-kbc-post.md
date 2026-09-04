# T516 S3: DeskPro Model 40 POST keyboard closure

## Observed ROM contract

The selected external Compaq Rev-E ROM performs keyboard `FFh`, consumes exactly
`FAh` then `AAh`, issues controller `AEh`, and treats any remaining output byte
as the `301-Keyboard Error` path.  Its error prompt is `(RESUME = "F1" KEY)`.
The trace reached the relevant ROM reads at `F:D27F`, `F:D2A5`, and `F:D2E4`.

## Repair

`FFh` now claims the keyboard's single BAT completion in the Core KBC owner.
A later controller line release cannot enqueue a second power-on `AAh`.
The independent line-release BAT path remains for firmware that does not issue
`FFh` (including the IBM 5170 contract).

The external Model 40 seed also contained a zero configuration checksum.  Its
`10h-2Dh` sum is `0169h`; the external seed now carries `2Eh=01h`, `2Fh=69h`
and manifest SHA-256 `0bae7ec0f94a611fb6e705597870b1871437dc784b63d9b4b2695198ddd43f48`.

## Verification

- `core-machine-kbc-controller-smoke`: pass, including inhibited `FFh` then
  `AEh` with one BAT.
- `vm-yaml-cmos-seed-smoke`: pass.
- External-YAML turbo boots: IBM 5170 1.2M and 360K, and DeskPro Model 40,
  each reach `ENTER=Continue` without injected input.
