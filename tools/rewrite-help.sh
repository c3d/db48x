#!/bin/bash
# Rewrite git history from a given commit, rebuilding help/db48x.md and
# help/db50x.md from their doc/ sources at each commit.  If the rebuilt
# files differ from what the commit recorded, amend the commit.
#
# Usage: tools/rewrite-help.sh <commit>
#   <commit>  The first commit to rewrite (e.g. a SHA, branch~N, tag).
#             Rewrites from <commit> to HEAD inclusive.

set -euo pipefail

if [ $# -ne 1 ]; then
    echo "Usage: $0 <commit>" >&2
    exit 1
fi

START_COMMIT="$1"

echo "Rewriting commits from ${START_COMMIT} to HEAD..."
echo "This will rebuild help/db48x.md and help/db50x.md at each commit."
echo ""

GIT_SEQUENCE_EDITOR=: git rebase -X ours --exec "${0/.sh/-helper.sh}" "${START_COMMIT}^"
