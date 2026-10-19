# M1 T3 S5: COP Register Boundary

## Scope

S5 consumes only the B07 access families observed by the bounded S4 owner-ROM
probe. It establishes one MyRaidenII Core COP owner for configuration/register
writes and the observed command-write range; it does not infer an operation,
return value, DMA transfer, timing model, or video/sound role.

## Owned Routes

The Core Board composes one private COP device. Its replacement routes are:

| Family | Physical range | S5 behavior |
| --- | --- | --- |
| Configuration | `0x428..0x42d` | Accept byte/word writes into COP-owned state. |
| Configuration | `0x432..0x43d` | Accept byte/word writes into COP-owned state. |
| Configuration | `0x444..0x445` | Accept byte/word writes into COP-owned state. |
| Configuration | `0x476..0x47f` | Accept byte/word writes into COP-owned state. |
| Configuration | `0x4a0..0x4ad` | Accept byte/word writes into COP-owned state. |
| Configuration | `0x4c0..0x4cd` | Accept byte/word writes into COP-owned state. |
| Command | `0x500..0x505` | Explicitly reject access pending an evidenced command model. |

All claimed reads return `LIB_STATUS_INVALID_STATE`. This is intentional: S5
does not invent undocumented configuration readback or COP results. Returning
`LIB_STATUS_UNSUPPORTED` would decline the replacement device and permit the
underlying ordinary RAM path, recreating the defect S4 observed. The command
range likewise returns `LIB_STATUS_INVALID_STATE` for writes and reads rather
than accepting or fabricating a command.

The sole range list supplies both decoder and route installation, so there is
no duplicate Board/COP mapping. Reset clears private COP storage; it retains
the installed routes. The Board remains the only composition owner and no
clock, scheduler, DMA path, Shared API, ROM data, or external code is added.

## Proof

The existing Board unit is extended rather than adding a parallel harness. It
proves every observed configuration family is a replacement provider, accepts
the owned write boundary, rejects reads explicitly, and that the adjacent
`0x426` remains ordinary RAM. It separately proves the command family is a
provider but rejects both read and write, and that reset preserves the device
route. Existing Core route machinery owns byte/word lane dispatch; S5 neither
duplicates that logic nor exposes test-only COP readback.

Focused `myraidenii.core/board-bus-smoke` passed for x64 and x86. Repository
unit qualification passed `687/687` for each architecture. Documentation
governance passed. S5 needs no executable because this is a ROM-free Core
boundary and no runnable Product or gameplay claim is made.

## Receiver

S6 must collect further bounded board observations and then model only the
actually required COP command/result/DMA semantics. The observed
`0x600..0x68c` and `0x6c0..0x6ce` ranges remain outside this COP receiver for
their Video/Bank owner; S5 does not absorb them.
