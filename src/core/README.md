# Core

Shared machine-core contracts and implementation live here. Public contract
headers stay beside their C implementation; private headers use `_impl.h`.

This module must not depend on product CLI behavior, firmware policy, or host
OS APIs.
