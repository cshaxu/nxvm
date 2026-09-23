# M5 T34 S4 - Battery Lifecycle Evidence

The persisted record is `MNSR`, version 1: it carries the Mapper number, a
64-bit identity over the accepted iNES image, and the declared PRG-RAM length
before its payload. Core exports/imports only the finite battery-backed MMC3
8 KiB profile. An absent, damaged, differently-versioned or differently
identified record is rejected without changing cartridge RAM.

App derives a sibling `.sav` path from the configured or inserted ROM path. It
attempts restoration only after the read-only cartridge is installed and
attempts a dirty save only after the Common shutdown boundary. Neither a
missing save nor a non-battery cartridge makes startup or shutdown fail.

The Core persistence smoke writes a battery record, changes the live RAM and
restores it, then proves an otherwise-valid different ROM cannot consume that
record. Focused cartridge, Mapper-4 and persistence tests
pass on x64 and x86.
