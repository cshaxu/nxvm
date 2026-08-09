# System Architecture

This is the current high-level architecture authority. Apply the local
[Architecture Rules](../rules/ARCHITECTURE.md) when changing it. Detailed M5
contracts, migration rationale, and hardware notes are supporting material
indexed in [etc/README.md](../etc/README.md); they cannot override this file.

## Product Shape

ntvdm64 has two product forms and three planned reusable components:

| Form or component | Purpose | Current state |
| --- | --- | --- |
| `nxvm.exe` / `vm` | Bootable whole-machine VM with the retained NXVM Console and debugger. | Runnable product. |
| `core` | Product-neutral machine, platform, and reusable product tooling. | Shared source foundation; not yet a separate artifact. |
| `mantle` | Policy-free VDM session composition over core. | Future component. |
| `dos` | Owned DOS implementation independent of the other components. | Future component. |
| `nxvdm.exe` / `vdm` | Non-bootable DOS application runner over mantle and dos. | Future product; only a non-runnable skeleton exists. |

The medium-term build targets are `core.dll`, `mantle.dll`, and `dos.dll`.
They are architectural commitments, not current release artifacts.

## Modules, Ownership, And Assembly

`core` contains `machine`, `platform`, and `product` modules plus the neutral
`utils` foundation. `vm` and `vdm` each use `machine`, `platform`, `product`,
and `profile` modules. `mantle` uses `machine`, `platform`, and `product`;
`dos` may use its own `machine`, `platform`, `product`, and `profile` modules.

```text
vm -------> core
mantle ---> core
vdm ------> mantle + dos
dos ------> (independent)
```

`core` owns generic guest-machine behavior and policy-free host abstractions.
It never depends on `vm`, `mantle`, `dos`, or `vdm`.

`vm` owns bootable-machine composition, BIOS/POST boot policy, VM profiles, and
the retained NXVM product experience. `mantle` owns reusable VDM composition,
but no DOS or host-policy decision. `dos` owns DOS behavior without depending
on another product component. `vdm` owns application-runner UX and combines an
admitted mantle session with an admitted DOS implementation.

The `vm` and `mantle` roots assemble their concrete sessions. The `vdm` root
selects application-runner UX and binds mantle to dos. Product-root composition
is where the declared machine, platform, product, and profile capabilities are
combined.

## Product And Host Boundary

`nxvm.exe` remains a bootable VM and retains its interactive Console; it does
not gain a replacement process CLI. `nxvdm.exe` will provide the approved DOS
application-runner interaction, display, cancellation, and debugger behavior
defined in [UI.md](UI.md).

Platform integrations report through opaque core contracts. Host policy and
guest-state mutation occur at the owning product composition boundary, never
inside a generic platform implementation.

## Research And Distribution Boundary

The owned DOS backend is the default NXVDM direction. External VDM/DOS and
historical component research can establish neutral capability requirements,
but cannot become a source import, ABI, backend, runtime dependency, or release
input without a separately approved implementation decision. Microsoft binaries
and third-party firmware remain user-supplied research material, never bundled
product inputs.

## Current Scope

M5 retains the runnable NXVM foundation while converging its shared-core
boundaries. `mantle`, `dos`, and `nxvdm.exe` are not current runtime products.
The milestone sequence and exit goals are defined only by [ROADMAP.md](ROADMAP.md).
