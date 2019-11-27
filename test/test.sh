#!/bin/bash

path=$(dirname $(dirname $(realpath $0)))
#compile helloworld:
$path/bin/bfpp -i $path/common/helloworld.bfk -o $path/helloworld.out || exit 1
$path/bin/bfrun -f $path/helloworld.out | grep -q 'Hello World!' && exit 0 || exit 1
