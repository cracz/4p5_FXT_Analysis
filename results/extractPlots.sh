#!/bin/bash

shopt -s nullglob

FILES=(*_0.picoDst.result.root)    #All of the first output files for any different jobs
NFILES=${#FILES[@]}

if [ $NFILES -eq 0 ]; then
    echo "No analyzer result files found in this directory."
fi

RUN=false

# Extract possible job IDs and ask which one is correct
for fname in "${FILES[@]}" ; do
    FULLPATH=${fname}
    JOBID=${FULLPATH%_0.picoDst.result.root}

    echo
    read -p "Extract plots from job ${JOBID##*/} ? [y||n] " choice
    
    if [ "$choice" = "y" ]; then 
	RUN=true
	FILES=(${JOBID}_*.picoDst.result.root)    #All of the output files for the desired job
	NFILES=${#FILES[@]}
	break
    elif [ "$choice" = "n" ]; then
	continue
    fi
done


# If job ID is found, run programs with that.
if "${RUN}"; then
    root -l -b -q combine.cxx\(\"$JOBID\",$NFILES\)
    root -l -b -q plot.cxx\(\"$JOBID\"\)
else
    echo
    echo "Nothing to do."
    echo
fi
