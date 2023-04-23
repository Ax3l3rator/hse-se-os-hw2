#! /usr/bin/bash

function ctrl_c(){
  echo "destroying"
  for fpid in ${fpids[@]}
  do
  kill -s INT $fpid 
  done
  return
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