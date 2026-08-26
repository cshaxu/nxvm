# M5 VM Profile Resolver Kernel

## Purpose

Establish the sole VM-side composition mechanism before any AT machine is
connected: immutable declarative profile data, one-parent typed patches,
provenance, validation, a frozen `vm_resolved_profile`, and one copied
Core-machine-plan construction route.

## Scope And Ownership

VM owns profile identity, inheritance, provenance and session policy; Core
owns all mutable execution, device and guest-time state. The kernel accepts
only neutral copied Core contract IDs and immutable field values. It rejects
conflicting ownership, invalid topology/dependencies, unavailable contracts,
invalid windows/routes and post-freeze mutation. It exposes neither profile
callbacks nor Core internals and creates no timing, scheduler or device path.

The kernel is proven with bounded neutral fixtures and negative validation
tests. It does not register, migrate, rename or alter any runnable IBM 5170,
DeskPro or `default-at` production profile; those are three following,
separate connection tasks.

## Dependencies And Completion

Consumes the accepted machine-neutral Core unified guest-time axis and VM
boundary. The subsequent PC/AT 5170 root, DeskPro child and `default-at` child
candidates each use this exact production kernel and delete their former direct
composition route. Completion requires resolver/immutability/conflict tests,
one copied-plan handoff, no machine-specific resolver branch, and no second
VM-to-Core configuration path.

## Non-goals And Stop Conditions

Do not select a machine, add a profile, change firmware/media, implement YAML,
create a generic compatibility profile, implement timing/device behavior, or
perform L4 work. Stop if a necessary Core value cannot remain neutral and
copied, or if proving the kernel requires a machine-specific production branch.
