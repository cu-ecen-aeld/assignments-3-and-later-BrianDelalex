#!/bin/bash

writefile=$1
writestr=$2

if [ "$#" -ne 2 ]; then
    echo "Usage: ./writer.sh writefile writestr"
    exit 1
fi

dirname=$(dirname $writefile)

if ! [ -d $dirname ]; then
    mkdir -p $dirname
fi

echo $writestr > $writefile