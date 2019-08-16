#!/bin/bash

if [ $# -ne 1 ]; then
    echo
    echo "Usage: ./fineHoles.sh [expected # of files]"
    echo "Be sure to first remove all job IDs with the following command:"
    echo "rename [job ID]_ \"\" *"
    exit
fi

NFILES=$1

if [ "$NFILES" -le 0 ]; then
    echo "Argument can't be less than or equal to zero."
    echo
    exit
fi

FILENUM="0"

while [ $FILENUM -lt $NFILES ]; do
    FILE="${FILENUM}.picoDst.result.root"

    if [ ! -e "$FILE" ]; then
	echo "File with index $FILENUM is missing!"
    fi

    FILENUM=$[$FILENUM+1]
done
