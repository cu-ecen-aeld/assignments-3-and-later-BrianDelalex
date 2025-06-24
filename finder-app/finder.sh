#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: ./finder.sh filesdir searchstr"
    exit 1
fi

filesdir=$1
searchstr=$2

if ! [ -d $filesdir ]; then 
    echo "filesdir is not a directory"
    exit 1
fi

x=$(find $filesdir -type f | wc -l)
y=$(grep $searchstr -R $filesdir | wc -l)
echo "The number of files are $x and the number of matching lines are $y"