#!/bin/bash
# Build and test every commit in a range, stopping on first failure so
# you can fix it and `git rebase --continue`.  Keeps a branch bisectable.
#
# Usage: tools/all-green.sh <since>          # from <since> to HEAD
#        tools/all-green.sh <since> <until>  # from <since> to <until>

set -euo pipefail

if [ $# -lt 1 ] || [ $# -gt 3 ]; then
    echo "Usage: $0 <since> [<until>] [<test>]" >&2
    exit 1
fi

SINCE="$1"
UNTIL="${2:-HEAD}"
TEST="${3:-all}"

NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 8)

echo "Building and testing every commit from ${SINCE} to ${UNTIL}"
echo "Using ${NPROC} parallel jobs"
echo ""

if [ $(uname) == "Darwin" ]; then
    DB48X=./db48x.app/Contents/macOS/db48x
    DB50X=./db50x.app/Contents/macOS/db50x
else
    DB48X=./db48x
    DB50X=./db50x
fi

git rebase --exec "${0/.sh/-test.sh} ${TEST}" "${SINCE}^" "${UNTIL}"
