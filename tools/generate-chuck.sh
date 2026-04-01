#!/bin/bash
echo "static const char *chuckles[] = {"

for I in {1..42}; do

    (fortune -a $HOME/.oh-my-zsh/plugins/chucknorris/fortunes 2>/dev/null ||
         echo "Chuck Norris is watching you, $(git config get user.name)") |
        sed -e 's/"/\\"/g' -e 's/\n/\\n/g' |
        sed -e 's/.*/    "&",/g'
done
echo "};"
