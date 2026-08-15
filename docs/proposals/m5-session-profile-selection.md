# NXVM Session Profile Selection

## Purpose

Expose the already-admitted VM profile descriptors through the retained NXVM
Console so a user can deliberately create the strict IBM PC/AT 5170 Model
339/Type 3 session instead of accidentally creating the configurable generic
PC/AT session with a `--cpu 80286` override.

## Scope

`SESSION OPEN` with no options presents the currently supported choices and
accepts a short numeric choice.  The product also accepts an explicit,
scriptable `--profile <name>` option.  The initial candidates are the generic
default PC/AT and `ibm-5170-model-339`; session listings and opening feedback
identify the selected profile.  The Model-339 selection retains its existing
identity constraints: its CPU, memory and no-HDD contract cannot be changed by
generic session-open overrides.

## Non-goals and completion

This is a product-entry task, not a new machine, firmware, ROM, media, device,
or timing implementation.  It must not add test-only selection paths, change
the default no-option session created during product startup, import external
assets, or start DeskPro, XT, or Windows work.  Complete it with console-flow
and provider parsing coverage, Model-339 constraint regressions, the current
gate, documentation governance, and an evidence record that gives exact user
steps for creating each available profile.
