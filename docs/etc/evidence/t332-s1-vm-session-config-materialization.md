# T332 S1: VM Session Configuration Materialization

## Construction Contract

`vm_session_create` now has one private materialization sequence:

1. `vm_session_materialize_profile_core_config` copies every
   `core_machine_config` field from the selected PC/AT profile;
2. when a caller supplies `vm_session_config`,
   `vm_session_apply_core_config_overrides` replaces only its existing explicit
   choices: memory capacity, CPU profile, and FPU profile; and
3. the resulting single config is consumed by `vm_session_initialize` and then
   `core_machine_create`.

The retained config copy, image paths, device-creation options, and boot choice
retain their existing ownership and behavior. The helpers are `static` in the
VM composition source; no public or cross-module interface was added.

## Focused Evidence

The retained `vm-session-initialization-atomicity-smoke` now creates both a
default session and an explicit 32-MiB/80286/8087 session. It proves the
default memory/CPU/FPU values equal the profile, the configured three values
equal the caller request and retained config, and both instances retain the
same profile-derived ticks, instruction-timing struct, clock-plan struct, and
all three KBC timing fields. It also reads the actual constructed core's
memory/CPU/FPU profile, proving the materialized values cross the composition
boundary rather than merely remaining in the session mirror. It retains the
existing core, firmware, controller, and recovery failure checks.

The smoke retains `M5:T300:S3:SESSION-INITIALIZATION-ATOMICITY:OK` and adds
`M5:T332:S1:SESSION-CONFIG-MATERIALIZATION:OK`.

The T332 developer artifact is `vm-0-5-0332` at
`build/output/nxvm_0_5_0332.exe`, SHA-256
`1DF13B58B316026C233127B9407A03003C8B997221B431BB321F01E4D0C54A7E`.

## Similar-Issue Sweep

The source sweep found no second VM composition default-versus-override
materialization branch. The test's local `initialize_config` remains a
deliberate internal-fixture constructor for injecting `vm_session_initialize`
failures before a session can become active; it is not a second production path.
Its broader setup/cleanup shape transfers to T332 S3 rather than exporting a
test helper through production code.

## Boundary

S1 does not alter lifecycle rollback, provider/media creation, image handling,
public configuration ABI, core/device policy, or test fixture lifecycle. Those
remain T332 S2 and S3 work.
