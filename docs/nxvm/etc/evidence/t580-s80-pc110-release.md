# T580 S80 — PC110 build and deployment

The admitted TFT and derived-DSTN bindings now each have one formal product
target. They reuse the existing PC deployment function and validated external
firmware/font inputs. The former preview targets were removed; product and
CMOS tests use the formal targets. No runtime firmware selection or fallback
was added.

Both cached x86 and x64 builds produced the four owner-selected files under
`assets/mypc110`. Their PE widths, lengths and SHA-256 identities are in the
[artifact record](t580-s80-pc110-artifacts.json). Tests checked that each
deployed file is byte-identical to its build target, contains its correct
full BIOS and font, excludes the other panel BIOS, and rejects a deliberately
swapped BIOS expectation for the expected reason. The external BIOS and font
bytes and generated source remain outside tracked source.

On each architecture, eight selected tests passed: the two product
composition units, two real-process CMOS/reset integrations, two artifact
firmware checks and two swapped-firmware negative checks. The artifact-root
and NXVM documentation-governance gates passed. The adjacent INI retained its
pre-build SHA-256
`EC0A054B825906BD3D9B0E20AC6C3ED154ADFD988E00A54F5469084EF9968950`.
The user-owned CMOS filenames remain ignored.

S78/S79 separately recorded real firmware, DOS and input evidence for both
variants and host widths. These four deployed EXEs still require the T580
full-repository, runtime product, desktop/RDP and remaining quality closure
checks; this artifact delivery is not itself final acceptance of T580.
