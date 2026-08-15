# T380 S1: Session Profile Selection

`M5:T380:S1:SESSION-PROFILE-SELECTION:OK`

## Result

The retained NXVM Console now makes the two currently supported session
profiles discoverable at `SESSION OPEN`:

```text
1. default-pc-at       Configurable generic PC/AT (default)
2. ibm-5170-model-339 IBM PC/AT 5170 Model 339/Type 3
```

Pressing `1`, `2`, or Enter selects the corresponding profile.  A noninteractive
user may instead use `SESSION OPEN --profile default-pc-at` or `SESSION OPEN
--profile ibm-5170-model-339`; the numbers also work in that option.  A newly
opened session becomes the selected session, and `SESSION LIST` plus `INFO`
display its profile, effective CPU and memory amount.

The Model-339 selection is descriptor-owned and remains fixed at 80286, 512
KB and no HDC.  `--cpu` and `--fpu` are accepted only with `default-pc-at`;
combining them with Model-339 is rejected as unavailable.  No ROM, media,
firmware, device or timing behavior changed.

## Proof

`vm-product-console-lifecycle-smoke` drives `SESSION OPEN`, selects `2`, then
confirms the newly selected session is selected and reports
`profile=ibm-5170-model-339 cpu=80286 fpu=none`.  `vm-session-profile-smoke`
proves both the name and numeric direct options, retained default profile
output, and generic-override rejection.  The Model-339 composition smoke
remains green.

Focused replay and the full current gate were run from a clean MSYS2 UCRT64
GCC 16.2 build directory.  The first full gate exposed only host PATH lookup
and an active-packet schema issue; both were corrected, then the retry passed.

## Developer artifact

T380 builds `build/output/nxvm_0_5_0380.exe`.

| SHA-256 | Source commit |
| --- | --- |
| `35B25B9497ADBFDE4B3E53FE9C78E2277732D9B8612A96EEE9A7F14E5F7C4D4A` | recorded by the implementation commit |
