# Contributing

Read the [Documentation Guide](docs/README.md) and the active
[Project Status](docs/STATUS.md) before proposing or changing work. The design
and rule documents linked there are the implementation authorities; this file
does not restate them.

## Change Submission

- Follow [Architecture Rules](docs/rules/ARCHITECTURE.md),
  [Coding Rules](docs/rules/CODING.md), and
  [Execution Rules](docs/rules/EXECUTION.md).
- Follow the [source and research procedure](docs/etc/operations/policy/source-policy.md)
  before importing, deriving, or packaging third-party source, firmware, guest
  media, or Microsoft material.
- Treat research and optional local tools as non-product inputs unless an
  approved task explicitly admits them.

## Review Record

Every change records:

- affected ownership boundary and user-visible behavior;
- source provenance, license, and redistributability impact when applicable;
- tests and evidence added or updated; and
- any deferred issue, owner-approved exception, or research boundary.

## Commits And Tracking

Use the identifier, commit-subject, artifact, cleanup, and closure rules in
[Execution Rules](docs/rules/EXECUTION.md). Do not allocate a task identifier
or treat a local developer artifact as release evidence outside that policy.
