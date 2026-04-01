#!/bin/sh
# Copy the generated help files to the DB48X simulator cache
CACHE=~/.local/share/DB48X/DB48X/help
cp help/db48x.md help/db48x.idx "$CACHE/"
echo "Copied db48x.md and db48x.idx to $CACHE"
