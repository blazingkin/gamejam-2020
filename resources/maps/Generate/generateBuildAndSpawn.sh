#!/bin/bash

if [[ $# -lt 3 ]]
then
    echo "Usage: generateFull.sh fileSuffix numBuildings numSpawners"
else
    python3 buildings.py $1 $2

    python3 spawners.py $1 $3
fi