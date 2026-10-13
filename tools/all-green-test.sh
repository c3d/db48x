#!/bin/bash

set -euo pipefail

TEST="${1:-all}"

if [ $(uname) == "Darwin" ]; then
    DB48X=./db48x.app/Contents/macOS/db48x
    DB50X=./db50x.app/Contents/macOS/db50x
else
    DB48X=./db48x
    DB50X=./db50x
fi

function die()
{
    echo "ERROR: $1"
    exit 1
}

make -j $(nproc) fw || make fw          || die "Cannot build firmware"
git add src/dm{32,42}/qspi_crc.h        || die "Cannot add CRC value"
git commit --amend --no-edit            || die "Cannot commit"
make -j $(nproc) sims                   || die "Cannot build simulators"
${DB48X} -I -Z -H -T${TEST}             || die "DB48x tests failed"
${DB50X} -I -Z -H -T${TEST}             || die "DB50x tests failed"
