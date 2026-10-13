#!/bin/bash
set -euo pipefail

# Rebuild help/<name>.md from doc/ sources, matching the Makefile pipeline.
# Args: name model  (e.g. "db48x dm42" or "db50x dm32")
rm -f help/db{48,50}x.{md,idx}
make .prebuild color-dm32-.prebuild
git add help/db{48,50}x.md
git commit --amend --no-edit
