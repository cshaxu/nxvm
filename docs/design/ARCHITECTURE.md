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
| `mantle` | Policy-free VDM session composition over core and admitted runtime adapters. | Future component. |
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
including the neutral adapter boundary for an owned or separately admitted
runtime, but no DOS or host-policy decision. `dos` owns DOS behavior without
depending on another product component. `vdm` owns application-runner UX and
combines an admitted mantle session with an admitted DOS implementation.

The `vm` and `mantle` roots assemble their concrete sessions. The `vdm` root
selects application-runner UX and binds mantle to dos. Product-root composition
is where the declared machine, platform, product, and profile capabilities are
combined.

## Product And Host Boundary

Platform integrations report through opaque core contracts. Host policy and
guest-state mutation occur at the owning product composition boundary, never
inside a generic platform implementation.

Native and WASM hosts share these component boundaries. A future TypeScript web
product layer sits above the WASM platform/product adaptation; it does not move
browser, network, or storage policy into generic machine behavior.

## Runtime Admission Boundary

The owned DOS backend is the default NXVDM direction. A separately admitted
external VDM/DOS implementation is isolated behind a dedicated adapter and
does not become a shared public ABI, default runtime dependency, or release
input. Source, firmware, research, and redistribution procedures are defined
by [Architecture Rules](../rules/ARCHITECTURE.md) and the indexed
[source policy](../etc/governance/source-policy.md).

Current delivery state and staged implementation goals are defined only by
[ROADMAP.md](ROADMAP.md) and [STATUS.md](../STATUS.md).
