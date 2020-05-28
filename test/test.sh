#!/bin/bash

path="$(dirname "$(dirname "$(realpath $0)")")"


function nope {
  return
} 

#compile helloworld:
$path/bin/bfpp -i "$path/common/helloworld.bfk" -o "$path/helloworld.out" || exit 1
$path/bin/bfrun -f "$path/helloworld.out" | grep -q 'Hello World!' || exit 1

$path/bin/bfpp -i "$path/common/pi.bfk" -o "$path/pi.out" || exit 1
$path/bin/bfrun -f "$path/pi.out" | grep -q '3.141' || exit 1

#check dpcrun:
$path/bin/dpcrun -f "$path/common/helloworld.bfk" | grep -q 'Hello World!' || exit 1

$path/bin/dpcrun -f "$path/common/pi.bfk" | grep -q '3.141' || exit 1
exit 0

