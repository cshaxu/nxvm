# M5 T539 S69 P1 -- Shared Video Aperture Compiler Gate

`x86_video_active_ega_aperture()` is a private Shared helper. Previously it
returned without writing its out parameters for invalid arguments, while its
three callers immediately consumed those locals. Valid callers always provide
an adapter, but the contract was not expressible to the compiler and a clean
x86 build correctly rejected the possible uninitialized use.

P1 changes the helper to return `lib_bool`. It returns `LIB_FALSE` for invalid
arguments and `LIB_TRUE` after retaining the existing aperture selection.

| Caller | Failure result | Valid-path behavior |
| --- | --- | --- |
| `x86_video_ega_planar_offset()` | offset `0u` | Existing aperture-to-planar offset calculation unchanged. |
| `x86_video_ega_write_observer()` | no dirty observation | Existing selected-aperture dirty observation unchanged. |
| `x86_video_ega_aperture_contains()` | `LIB_FALSE` | Existing aperture containment calculation unchanged. |

The helper remains private: no public ABI, profile mapping, EGA/VGA behavior,
firmware, asset, INI or EXE input changed.

## Verification

- `x86-video` built cleanly with `-Werror` on x86 and x64.
- Focused `x86.ega_sequencer`, `x86.ega_controller`, `x86.compaq_ega_s6` and
  `x86.ega_planar` passed on x86 and x64.
- Complete repository-only unit suites passed 426/426 on x86 in 86.82 seconds
  and 426/426 on x64 in 83.89 seconds.
- T344 registration/historical shapes, T332 lifecycle, VM-machine lifecycle,
  Core CPU/PIC authority, T388 lexeme/physical eligibility, documentation
  governance and `git diff --check` passed on both widths.
- MyNES was not built or modified; no product EXE rebuild is required.
