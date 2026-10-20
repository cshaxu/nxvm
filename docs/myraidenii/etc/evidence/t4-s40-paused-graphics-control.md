# M1 T4 S40 — Paused Graphics Mutation Control

## Purpose

S40 tested the public MAME Lua pause and video-update facilities as a stricter
black-box control for S37's unpaired mutation observation. It did not read or
retain game memory, registers, pixels, screenshots, source or debugger output.

## Paused Experiment

At an identical post-startup frame, the transient external script paused the
session, held a baseline screen only in process, changed one byte of the
external tile region to synthetic `0xff`, requested a public video update and
compared the two in-process screen strings. Both repeated runs emitted only:

```text
320 × 240; changed pixels 0; baseline checksum 1079166174;
later checksum 1079166174
```

The same no-difference result occurred when the whole external tile region was
first replaced by zeros. This establishes that the public pause/update route
does not itself cause the required graphics-region state to be consumed by the
reference presentation.

## Advancing Control

A separate bounded two-frame experiment captured a baseline in process at one
frame, optionally wrote the same synthetic byte, then compared at the second
frame. Each of two mutation runs and two no-mutation controls emitted the same
neutral summary:

```text
320 × 240; changed pixels 37082; difference checksum 1493930203;
later checksum 852859541
```

The equality is the missing control that S37 did not have. It proves the
earlier changed screen checksum was caused by ordinary reference-frame
advancement, not established by the selected byte mutation.

## Disposition

S37's causal mutation conclusion is superseded. No current black-box result
establishes a graphics element's size, location, bitplane ordering, attributes,
palette, priority, scroll or decryption. The paused route is not a usable
isolation mechanism, and the advancing route remains time-confounded.

All scripts stayed under the disposable S40 build directory and were deleted
after this record. MAME configuration remained external under
`nxvm-assets/media-myraidenii/raiden2/mame-config/`; no repository-root `cfg/`
or protected payload remains.
