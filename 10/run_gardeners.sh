#! /usr/bin/bash

function ctrl_c() {
    ./stopper
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