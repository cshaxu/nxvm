# Canonical Breakdown

## M0 Governance And Laboratory

Establish repository policy, reference baselines, generated probe conventions,
and the C/Windows build decision.

- T1 Governance and laboratory contract: S1 establishes source, asset, evidence,
  architecture, tracking, and license boundaries; S2 establishes CMake, the
  Visual Studio toolchain contract, and deterministic generated probes.
- T2 Reference baseline discipline: S1 pins the permitted NXVM reference and
  records its license boundary; S2 records the behavioral-only NTVDMx64 and
  external WineVDM boundaries.
- T3 M1 entry gate: S1 reviews that no runtime, protected asset, or third-party
  source entered M0 and that the M1 COM launch contract is explicit.

Completion: no code dependency is imported, boundaries are reviewable, the M0
probe generator produces a manifest-verified COM input, and M1 has an
executable acceptance contract.

## M1 DOS Launch Spine

T1 implements CPU/memory integration sufficient for a bounded real-mode COM
probe. T2 creates synthetic DOS process state and supports INT 20h plus INT
21h AH=09h and AH=4Ch. Completion: a generated COM program prints a marker and
returns a recorded exit code via `ntvdm64 run`.

## M2 DOS Host Bridge

T1 adds safe COM/MZ classification and MZ relocation/loading. T2 adds DOS path,
handle, file, directory, environment, and child-process services. T3 adds text
console, keyboard, and deterministic diagnostics. Completion: a defined corpus
of redistributable or generated probes passes from a Windows terminal.

## M3 Interactive Compatibility

Add profile-selected mouse, video surfaces, timer behavior, and selected
XMS/EMS contracts. Completion: each enabled profile has evidence and preserves
the M1-M2 corpus.

## M4 Extended Programs And Win16 Routing

Add only scoped DPMI/DOS-extender support justified by a target corpus. Detect
NE executables and invoke an external WineVDM backend. Completion: routing is
diagnostic, configurable, and does not make the project a GPL combined work.

## M5 Packaging And Shell Integration

Create reproducible package manifests, upgrade/uninstall behavior, security
documentation, and optional per-user shell integration. Completion: explicit
launch works without integration; integration is reversible and does not
replace global `.exe` association or install global hooks.
