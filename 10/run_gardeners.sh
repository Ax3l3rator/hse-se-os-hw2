#! /usr/bin/bash

function ctrl_c() {
    echo "destruction"
}

trap ctrl_c INT

for i in {0..1}
do 
  ./gardener $i &
  pids+=($!)
done



for pid in ${pids[@]}
do
  wait $pid 
done