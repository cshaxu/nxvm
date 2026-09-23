# Td S150: Profile Connection Order

`M5:Td:S150:PROFILE-CONNECTION-ORDER:OK`

## Decision

The owner requires an early, correct Core/VM structure followed by machine
connections in order: IBM 5170, DeskPro 386, then `default-at`. The queue now
places a machine-neutral Core guest-time boundary first and a machine-neutral
VM resolver kernel second. These two tasks establish the only reusable timing
and composition paths before a profile is connected.

## Resulting Order

1. Core unified guest-time axis and VM boundary.
2. VM profile resolver kernel.
3. PC/AT 5170 root connection.
4. DeskPro 386 child connection.
5. `default-at` child connection.
6. YAML/session selection of the completed built-in profiles.

The resolver kernel owns no selected machine data; each profile connection
only supplies immutable values and provenance through it. No task may recreate
the Core axis, Core/VM observation contract, resolver, profile patch route or
direct composition path.
