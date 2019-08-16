#!/bin/bash

shopt -s nullglob

FILES=(*.picoDst.result.root)
NFILES=${#FILES[*]}

if [ "$NFILES" -eq 0 ]; then
    echo "No files found."
    exit
fi

for fname in "${FILES[@]}" ; do
    mv "${fname}" job_"${fname}"
done
