#! /usr/bin/bash

function ctrl_c(){
  ./stopper
}

trap "ctrl_c" 2

for i in {0..39}
do 
  ./flower $i & 
  
  fpids+=($!)
done


for fpid in ${fpids[@]}
do
  wait $fpid 
done