# T559 S4 CMPXCHG

## Contract And Design

In-progress I3 implementation against S3 c1a04cffb and admission 9400450db.
The pinned Intel 486 family programmer's reference, pages 26-62--63,
defines AL/AX/EAX comparison with the destination, source transfer when
equal and destination transfer to the accumulator otherwise. Its note
explicitly requires a destination write cycle in both cases, writing the
original destination on comparison failure. This is not a read-only branch.
The source identity/hash remains recorded by T558/S1; no external code import.

Existing CPU metadata admits 0FB0/B1 only for the selected 486SX; the existing
ModRM scanner already covers both. LOCK dispatch admits their memory forms,
and the existing ModRM owner rejects locked registers. Two width-family
handlers preserve the neighboring instruction style, use the existing CMP
flags, and check write access before loading a memory destination. Cached
source and effective address remain stable before any accumulator write.
Both outcomes use the existing destination-write path. Existing exception
finalization owns CPU rollback on failed writes; no claim is made to undo
arbitrary accepted device side effects.

The planned source surface is CPU instructions, metadata-test expectations,
one CPU-owned test and its registration/manifests. Estimated production
+40--100/-5--20 and tests +250--450/-0--20. No public API, persistent state,
second executor, Shared-six or MyNES change. Production 486 admission remains
closed; cache/TLB/WP/AC and full 486 timing retain their system receiver.

## Direct Test Matrix

- Byte/word/dword, default 16/32-bit code, all source/destination GPR
  encodings including high bytes and accumulator aliases, equal/unequal
  outcomes, independent subtraction flags and untouched architectural state.
- Memory operands in 16/32 addressing, all source encodings, LOCK on/off,
  both outcomes with exactly one write and correct original/replacement data.
- Protected and VM86 SIB forms; source also provides the effective address;
  writeback never recalculates the address from changed registers.
- Old-profile and locked-register rejection, metadata/preview agreement and
  truncated required-byte rejection without breaking early POP CS preview.
- Required-byte fetch/read/write failures on both outcomes, unchanged CPU
  and memory followed by successful retry with one accepted write.
- DS/SS bounds, nonwritable segment, missing/read-only CPL3 page, combined
  segment and page denial, saved fault state and write-class page errors on
  both outcomes; no accumulator or flags publication on failure.

The complete focused CPU batch passes on x64 and x86. Before final full-suite
qualification, review expanded arithmetic vectors, added accumulator-based
SIB writeback (including the unequal path that changes EAX), and injected
each of the four operand-byte read/write failures on both outcomes. These
additions pass both widths. Source and test actual-change review found no
additional execution owner, state or API; the inherited RMW residual remains
the S3 TODO rather than an undocumented broad rewrite. Search scope includes
CPU CMPXCHG/B0/B1 metadata, LOCK, dispatch and timing references; timing's
primary immediate-MOV opcode ranges are unrelated, and full 486 timing is
still the named system receiver.

Final complete repository units pass 595/595 on each width: x64 in 219.93
seconds and x86 in 43.88 seconds. The logs are
`build/t559-s4-x64-unit.log` and its x86 counterpart. This includes all five
Apps and Core plus Shared unit groups, including three registered desktop
unit cases per width; it is not manual desktop/RDP or external guest boot
qualification. The latter remains a T-level gate.

Production changes are +46/-3 (net +43) in the existing CPU instruction
file. Test C/CMake changes are +428/-1 (net +427), including the 425-line
new owner test; manifests, documents and binaries are excluded. The two
handlers retain the neighboring byte/operand-size split and existing ALU,
ModRM, logical-memory and exception-finalization owners. No public function,
type, persistent state, compatibility path or helper framework was added.

Eight 0559 PC EXEs are rebuilt and deployed. Each passed the real-product
INI identity integration once, plus PE-width and embedded version checks.
Both Ninja graphs are restored to default NXVM. Shared-six, MyNES and all
owner INI/media/ROM inputs remain unchanged. Core manifests, documentation
governance and whitespace checks pass. Coordinator acceptance remains pending.

| Product artifact under `assets/` | SHA-256 |
| --- | --- |
| `my5160/my5160_0_5_0559_x64.exe` | `BD8557EC8AC1CCE633A1B69AAFD622662A6D4207A636C26C83466F470535454F` |
| `my5160/my5160_0_5_0559_x86.exe` | `33F09EE9E0BAF7E1A5B29C82ED6C7234D631A15280667485DBFD64476C1FBA2D` |
| `my5170/my5170_0_5_0559_x64.exe` | `4B8E80695026B1F4D42FE9FA84B6E33066CD93B7A4740AA5C12D9F2C1A1EB6E1` |
| `my5170/my5170_0_5_0559_x86.exe` | `43E6A678B4C2A328738254EA8E1D13348003062E19FC2163B3C2ED4AC0E6275E` |
| `mydeskpro386/mydeskpro386_0_5_0559_x64.exe` | `517B39EB84C33F66E4B9AB49DDB11B7C0444510A867D17C0C86E0BA279241FCE` |
| `mydeskpro386/mydeskpro386_0_5_0559_x86.exe` | `05AAE88D227F9CB99CD1C993BE55A6B5AED59676B6A8FC0BC3C8DE742C8E0F9F` |
| `nxvm/nxvm_0_5_0559_x64.exe` | `ECC3068448173867C1C9258A00CFD89CDEB2BAC4D8AD96EAD5F254ABCB43E43B` |
| `nxvm/nxvm_0_5_0559_x86.exe` | `B6F4DE50491F604DCFDE2F9F3EFD136DA270CAF90F02228DE2F12FAACD5A746B` |
