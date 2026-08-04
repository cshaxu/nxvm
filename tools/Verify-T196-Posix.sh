#!/usr/bin/env sh
set -eu

build_dir="${1:-build/t196-posix}"

cmake -S . -B "$build_dir" -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build "$build_dir" --target vm-0-5-0196 \
    vm-platform-run-handle-smoke vm-platform-linux-run-handle-smoke

# script supplies the controlling terminal required by ncurses and propagates
# the smoke result with -e. The caller must run this from a POSIX terminal.
TERM="${TERM:-xterm}" script -q -e -c \
    "./$build_dir/vm-platform-linux-run-handle-smoke" /dev/null

printf '%s\n' "M5:T196:S3:POSIX-HARNESS:OK"
