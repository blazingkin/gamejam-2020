#!/bin/bash

if [[ $# -lt 4 ]] || [[ $# -gt 5 ]]
then
    echo "Usage: generateFull.sh fileSuffix maxDistFromRoad numBuildings numSpawners [runSilent]"
else
    if [[ $# -eq 5 ]]
    then
        python3 roadGradient.py $1 $2 >/dev/null
    else
        python3 roadGradient.py $1 $2
    fi

    python3 buildings.py $1 $3

    python3 spawners.py $1 $4
fi