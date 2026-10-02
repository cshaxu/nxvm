# T540 S71: RAM alias and parity construction boundary

Baseline: accepted S70 P2 `d1fe5c0aa`. Intake reads machine_board.c,
machine_plan.c, memory_interface.c, memory.c, port_interface.c and the D4
memory adapter. Search all NXVM production for
`core_machine_memory_(register_mapping|enable_parity|release_parity)`.

## Complete receiving class

- Board construction installs one unselected high-reset RAM fallback for
  resolved 286/386 when installed RAM covers the lower 1 MiB. Board retains
  that address-decoding choice; Core owns checked map publication.
- Plan application installs its selected aliases into the same RAM. Move
  the existing copied three-field descriptor to the neutral memory interface.
  One batch uses the existing private validator and restores its starting
  count on failure; no duplicate map or raw storage crosses the boundary.
- Planar parity currently allocates Core parity directly, publishes a port,
  then directly releases parity on port failure. Use the existing public
  memory-route transaction with zero routes plus parity, followed by the
  existing owner removal on failed port publication. Board owns this paired
  construction sequence; Core owns allocation and destruction. Publish the
  board configured flag only after both preparations succeed.
- D4 already uses that memory-route transaction for routes/observer/parity.
  Its same owner token cannot be silently removed: the existing memory owner
  admission must reject a pre-existing registration before planar preparation.

Port construction's outer checkpoint/status/rollback and refresh timer port
writes are a distinct subsequent receiver. Firmware publication, provider
binding and physical relocation remain open; this S does not claim their cut.

## Required evidence

Synthetic unit inputs prove mapping behavior, failed-batch rollback, frozen
configuration rejection and parity port-conflict cleanup/retry. Independent
Core proof must remain linkable without board archives. Both complete unit
suites, gates and eight rebuilt products/one-shot boots precede acceptance.
No source, test, configuration or artifact of Shared/MyNES may change.

## Implementation checkpoint

The descriptor now belongs to memory_interface.h. The Core alias batch uses
the existing private mapping validator and restores mapping_count on any
failure; the two production callers no longer borrow executor_memory.
High-reset fallback still uses resolved CPU/RAM values, obtained through the
existing copied-value getters. Planar parity uses the existing memory-route
transaction and owner-removal rollback; its board configured flag remains
success-only. Oversized parity requests remain rejected by the original Core
allocation validator rather than duplicate board validation.

Independent Core and planar parity focused x64 regressions pass. New tests
cover invalid inputs, selected alias reads/writes, rollback over a prior
mapping, frozen registration and parity port conflict cleanup/retry. The
controller-authority gate scans all NXVM production for raw memory construction
outside its neutral owner and rejects executor_memory borrows in the board
constructor and plan. A full production search leaves
raw calls only in memory.c and memory_interface.c, their legitimate owner.
All existing same-owner hardware fixtures remain intact.

The status-table review found a stale S69 `Active` row despite its accepted
P2 and the current S71 packet. It is corrected to the actual accepted proof,
with explicit S70 accepted and S71 active rows; no second active S is implied.

The initial specialized run found that the old Port-B gate required Board's
retired raw allocation/release calls. Its receiving update keeps preparation
before port publication, rollback before board success publication, rejects
raw memory access, and checks Core's owner-qualified parity cleanup. A search
of all NXVM CMake gates finds the D4 gate already follows this Core contract;
no second obsolete gate remains. This is a verifier receiver correction, not
a production workaround or relaxed ordering requirement.

Git numstat counts nine source/test/gate paths: +186/-33, net +153. The
increase supplies one actual all-or-none owner transaction and regression
coverage, not a state mirror or forwarding-only abstraction. Production memory
storage and its range validator remain unchanged.

Complete units pass 470/470 on x64 (284.74 seconds) and x86 (73.19 seconds).
Both-width specialized gates and documentation/diff checks pass. The strict
compilation gate covers 402 rows (377 retained strict, 25 declared deferred);
dependency inventory remains the accepted 37-edge graph.

All eight Release products, matching probes and actual-source independent
Core executions pass. One-shot external boots reach `dos-prompt` for default
x64/x86 and `installer-running` for XT, AT and Model40 x64/x86. All eight
return zero. No repeated boot is used as acceptance. These are scoped headless
checkpoints with unchanged owner INIs/external overlay media, not native UI
manual verification or complete T540 integration acceptance. Retain
`build/s71-{build,unit,gates}-{x64,x86}.log` and
`build/s71-<profile>-<width>-{build,neutral}.log`, the two t540-s4 unit trees and
the eight t535-s4 Release receiving trees until this receiver closes.
Boot logs are `build/s71-<profile>-<width>-boot.log`. Shared, MyNES and owner
INIs have no tracked diff. External masters are not written.
Implementation is ready for the complete P1 delivery and actual coordinator
review; S71 acceptance follows separately.

## S71 artifact identities

All eight product post-build architecture checks pass. `objdump -h` confirms
no `.debug` sections. Products remain in their existing `assets/nxvm` profile
directories; INIs are unchanged. SHA-256:

| Product | Width | SHA-256 |
| --- | --- | --- |
| default | x64 | 553B0D0DF9FA621501C8FAF0BAF7E2E101E72FDDF74EAF6FA30F009CE5D82672 |
| default | x86 | AB4DF89FF1D498AE0EF0B8932E186A3C816924EA166F3945F8A44B1732664D09 |
| XT | x64 | 906B71657844D95E0E4AE9E58A4828FCEAE628F0B1D6B481AD36BCD322DFCF52 |
| XT | x86 | 77709C5ECED28AFA2EBFAAE049C53F3EE7C308E5199A0BF02C9FF2362A061D43 |
| AT | x64 | AA9180AABEAB714CC1AE6D5A19E4AD742F7FEC718BDFCBD46CB9B3C627BBD595 |
| AT | x86 | 108CB1B050A4C19C89932E6373195F070E17A96E12D9CE882A9441CE6EA7C186 |
| Model40 | x64 | 61885C23D37FB876D1FACD0718856604F581D487546EC46F88FCA21623584A75 |
| Model40 | x86 | EB120B0FEA4461D1369A4E5C4ABDC001CEFBBD793EE4FE1B44B8D8FB1E7CF3E3 |
