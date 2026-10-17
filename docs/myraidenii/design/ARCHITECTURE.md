# Architecture

## Ownership And Dependencies

MyRaidenII uses `src/myraidenii`, not `src/app-myraidenii`. Keep three real
responsibilities when implementation begins:

- `product`: entry, identity/version/banner, INI/assets policy, controls and
  injected command/debug semantics. It composes public Machine and Emulator
  contracts; it owns neither a host loop nor another lifecycle state machine.
- `machine`: one opaque Emulator driver, configuration/ROM transaction,
  input/frame/PCM adaptation, safe debug/snapshot operations and host pacing.
- `core`: deterministic board assembly, bus maps/banks, interrupts, COP,
  graphics, audio devices, controls/NVRAM and one guest timeline.

Dependency direction is Product -> Machine -> Core, with Product/Machine
using public Emulator/Lib capabilities. Emulator -> Lib remains unchanged.
Core hardware uses Lib Types only, apart from an explicitly qualified CPU
contract. No dependency on MyNES, the four PC Apps, X86 NXVM, X86 IBMPC
Product or PC boards is permitted. Product command callbacks access paused
machine data through Emulator's existing debug rendezvous, not private Core.

## Reuse Decisions And Real Gaps

Reuse Lib Types/Base/Storage/PCM/KVM and Emulator Product/Session/UI/Machine
where their existing contracts fit. T1 found the indexed graphics limit below;
M0's unconditional unchanged-reuse assumption is withdrawn, not implemented
as a Shared change. Product supplies the banner; Emulator owns Built on, fixed monitor
grammar, help formatting, prompt and lifecycle. One provider/router is injected.
Lib owns native handles, input decoding, presentation and actual audio output.

The current X86 Core CPU has no V30 profile. An 8086/386 profile is not a valid
substitute. Preferred direction is a separately reviewed V30 extension at the
existing shared CPU owner, qualified for NEC instructions, flags, entry,
mode/bus behavior and timing before board admission. The neutral X86 Core
Machine may supply execution/time only if its public bus/scheduler contract
fits this board without PC dependencies. If it does not, revise the design
before coding; do not add a second x86 interpreter or copy Core internals.
M0 and T1 research authorize no Shared source/API change. T1's inspected
memory routes, interrupt acknowledgement and attachment deadlines support
using neutral X86 Core Machine as the initial execution/timeline owner, after
V30 and its time qualification are added. Board hardware remains App-owned.
Do not construct another runner/time owner merely to avoid its public contract.

Z80, YM2151 and OKI sample devices are absent. They are deterministic board
mechanisms, not host PCM/MIDI services. Lib FM explicitly has no software
synthesizer. Initial implementations belong locally under Core, with cohesive
files rather than an invented shared-device framework. A future real second
consumer can justify separately approved extraction; Neo Geo is not a current
consumer or a reason to introduce placeholder packages.

Seibu COP/protection, sprite decode/decryption, banks, priorities, scroll/DMA,
sound latches and interrupt wiring belong to the board. No protection bypass,
ROM patch, scripted attract loop or game-PC special case substitutes for them.

## Execution, Frames And Audio

Emulator owns the single host execution worker and control queue. Main/sound
CPU and video/audio/COP deadlines advance on one deterministic board clock
with explicit rational conversions and stable tie ordering. Do not create a
sound CPU thread, independent host-time clock or frame-count timing shortcut.
The shared executor callback is serviced at bounded safe points; callbacks
never retain stale machine pointers across media/reset replacement.

Guest output flows from device state to complete copied Core output, Machine
frame/PCM conversion, Emulator publication and Lib presentation. Failed
conversion preserves the previous accepted frame; no ready output is distinct
from a real failure. Portrait orientation is a bounded output transform, not
a mutation of guest video RAM or forced Lib geometry. Reference native raster,
rotation, pixel aspect and host scaling remain distinct qualified facts.

Existing KVM graphics use 8-bit indices and 256 palette entries. The selected
board's reference palette has 2048 entries; lossless full-frame compatibility
is not established. The owner requires Lib unchanged and placed the proposed
RGB publication change in [TODO](../states/TODO.md). Core video must retain
correct board colours. Do not reduce colour fidelity, silently wrap indices or
invent another display path to claim full-game release. This does not block
independent CPU/board/sound work; full-colour presentation remains gated.

PCM synthesis/mixing advances from guest clocks, with an explicit bounded
sample queue and overflow/cancellation semantics. Lib's accepted-frame count
is authoritative, including partial failure; no replay of an accepted prefix.
Pacing may throttle produced guest work but never invent or skip guest ticks.

## Configuration And Resource Lifetime

One App-owned lowercase `myraidenii.ini` is resolved beside the EXE through
Lib Base/Storage. ROM paths resolve relative to that INI, not the process cwd.
The first fixed set uses a code-owned role/size/hash manifest and an external
ROM directory; no runtime machine registry or unrestricted board selector.
Missing/wrong inputs fail clearly before publishing a candidate. All acquired
data is validated before replacing live state; files stay read-only.

The supplied input is a ZIP, but current Lib Storage has no archive decoder.
Initial runtime loading uses validated individual ROM files in an external
directory; explicit offline preparation may unpack the supplied archive once.
Do not pretend Lib reads ZIP members or add an archive subsystem merely for
this product. Preparation must preserve source data and verified role identity.

Core owns guest NVRAM meaning. Product owns its path/persistence policy;
Machine performs storage at serialized boundaries. Snapshot support is not
assumed: until it covers both CPUs, devices, pending events and content
identity, shared save/load report Feature not implemented.

Stop/join the sole executor while sinks remain alive, retire/join UI producers,
then release wrappers, Machine and Core in the existing Emulator composition
order. Join/unbind failure retains ownership; no hidden recovery thread.
Debug, when implemented, requires the paused lease and explicit V30 debugger
qualification; current Intel disassembly is not proof of NEC support.
