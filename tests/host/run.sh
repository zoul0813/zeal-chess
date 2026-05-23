#!/bin/sh
set -eu

cc -std=c99 -Wall -Wextra \
    -Itests/host/include \
    -Isrc \
    src/chess.c \
    tests/host/test_chess.c \
    -o /tmp/zeal-chess-host-tests

/tmp/zeal-chess-host-tests
