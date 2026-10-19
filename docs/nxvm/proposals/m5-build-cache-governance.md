# Repository Build And Temporary-Output Governance

Owner-approved standalone M5 Td S181; no numeric task or product revision.
The active packet and delivery status belong to Current, not this proposal.

## Design

Root CMake includes one BuildEnvironment module before project configuration.
Ninja trees must resolve under build/.ninja, with stable compiler/architecture/
configuration names. Optional ccache uses a generated compiler launcher that
sets repository-local cache, temp and5G capacity for every actual compile.
Disabled configuration clears the launcher. Presets differ in cache selection,
not the directory of an otherwise identical compiler/configuration.

One Clear-Build tool accepts only explicit build-relative targets, previews
by default and preflights every target before deleting any. It rejects path
escape, cache roots/descendants, Git data and links. Sensitive file extensions
require exact reviewed SHA256 identities; caller must actually verify process
exit, not treat its acknowledgement switch as a process detector. Tests call
this real tool in-process. Configuration tests call real CMake, not a second
preset parser. The documentation checker remains unchanged.

Reconfigure new stable trees; never relocate an old CMake tree. Preserve
global ccache. Independently owned standalone component entry points retain
their portability; repository standalone tooling injects this root build
policy without adding an outer dependency to shared component sources.

## Migration And Verification

The audited60 old directories and3187 root files are enumerated in evidence.
Keep .ccache; create .ninja. Record required prior results and identities before
removing obsolete outputs. Preserve ambiguous configs/media copies and research
manuals outside the repository with hashes, never in either cache. Stop for
any Git checkout until provenance, clean state and local commits are checked.

Prove both real Release compiler graphs, no-op increments, cache-on/off reuse,
hostile caller environment, boundary rejection and cleanup safety. Run the
directly affected governance tests and all product documentation checks. Do
not regenerate release binaries or run native devices for a tooling-only task.
Record actual script/docs deltas, bytes removed, preserved inputs and all
unverified boundaries. Commit/push complete target-scoped delivery for review.
