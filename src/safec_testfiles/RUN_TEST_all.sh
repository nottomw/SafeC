#!/bin/bash

SCRIPT_NAME=$(basename "$0")

for i in `ls *.sh`;
do
    if [ "$i" != "$SCRIPT_NAME" ];
    then
        ./$i
    fi
done
