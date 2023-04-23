#! /usr/bin/bash

function ctrl_c(){
  ./stopper
  for fpid in ${fpids[@]}
  do
    if ps -p $fpid > /dev/null
    then
      kill -n 2 $fpid
    fi
  done
}

trap ctrl_c INT

for i in {0..39}
do 
  ./flower $i & 
  
  fpids+=($!)
done


for fpid in ${fpids[@]}
do
  wait $fpid 
done