#!/bin/bash

work=$1
Tus=(100000000 10000 5000 1000 500 100 50 20 10 7 5 3 1)
Tname=(baseline 10ms 5ms 1ms 500us 100us 50us 20us 10us 7us 5us 3us 1us)
# Tus=(100000000 5 3 1)
# Tname=(baseline 5us 3us 1us)
trial=33

folder="results/$work"
if [ ! -d "$folder" ]; then
	mkdir $folder
fi 

for ((i=0; i<${#Tus[@]}; i++)); do 
    tus=${Tus[i]}
    tname=${Tname[i]}
	echo $tname

	folder="results/$work/$tname"
	if [ ! -d "$folder" ]; then
		mkdir $folder
    fi 
       
	export INT_INTERVAL=$tus 
	for ((j=1; j<=trial; j++)); do
        echo trial $j
		result="$folder/$j"
        ./bench ../../server.config 1 1 $work >$result 
    done
done
