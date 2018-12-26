path=$(dirname $(dirname $(realpath $0)))
echo $path
#compile helloworld:
$path/bin/bfpp -i $path/common/helloworld.bfk -o $path/helloworld.out || exit 1
$path/bin/bfrun -i $path/helloworld.out > $path/result || exit 1
