#!/bin/bash
# Generate a Qt resource catalog
# Can't do that inline in the makefile because we have too many files
# for the Windows shell to cope with now

PREFIX=$1
PATTERN=$2
TGTPATH=$3

echo '<RCC>'
echo ' <qresource prefix="/'$PREFIX'">'

for PAT in $PATTERN
do
    find "$PREFIX" -name "$PAT"         | sort |
        while IFS= read -r I; do
	    J=${I#$PREFIX/}
	    E=$(printf '%s' "$J" | sed -e 's/&/\&amp;/g')
	    echo '  <file alias="'"$E"'">'"$TGTPATH/$E"'</file>'
        done
done
echo ' </qresource>'
echo '</RCC>'
