# M5 T34 Persistence And Release Design

T34 keeps machine semantics local: Core owns battery-backed cartridge RAM,
content identity and dirty state; App decides load/save at paused, stop, eject
and exit boundaries; Lib exposes a format-neutral atomic replacement primitive.
Common is not involved.

| S | Outcome |
| --- | --- |
| S1 | Audit and design this ownership, file identity and failure contract. |
| S2 | Add Lib atomic replacement with fault-injection proof. |
| S3 | Add Core battery RAM identity/import/export/dirty behavior and fixtures. |
| S4 | Connect App lifecycle/configuration save policy and verify replacement boundaries. |
| S5 | Generate a checked release manifest for the versioned x64/x86 EXEs and sole editable INI; prove reproducible inputs. |
| S6 | Independently audit T34; corrective S tasks if required. |

Save files use a versioned Core header containing a fixed magic, format version,
mapper/profile identity, PRG-RAM byte count and ROM-content digest. A missing or
invalid identity is ignored without changing the live cartridge. Lib writes a
same-directory temporary file, flushes/closes it, then replaces the destination;
any failure preserves the old destination. T34's initial format persists only
the declared battery profile. The release manifest records artifact hashes and
the configuration/build identity, never ROM paths or bytes.
