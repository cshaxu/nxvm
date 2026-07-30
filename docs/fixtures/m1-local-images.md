# M1 Local Image Fixtures

These owner-provided files are local, read-only M1 baseline inputs. They are
not committed, copied, distributed, or represented as redistributable assets.
Their contents and source path are intentionally not recorded here.

| Logical name | Size (bytes) | SHA-256 | Role |
| --- | ---: | --- | --- |
| `fdd.img` | 1,474,560 | `fadeb3a27c6a0e1cf582dde0b9aecb7e5d30678f2f967f2f4562f167cc0cb1d5` | Existing NXVM removable-disk baseline input. |
| `hdd.img` | 51,609,600 | `f4d1e81bc410bb9a7558667b7c3741a9664e84077a3774e73104cd24b631d688` | Existing NXVM hard-disk baseline input. |

Before an M1 run, the operator verifies both size and SHA-256. A mismatch,
missing fixture, or unclear legal basis stops that fixture run; it does not
authorize replacement media or a repository update. Evidence records state
only the logical name and hash used.
