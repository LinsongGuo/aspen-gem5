#!/bin/bash

work=$1
Tus=(100000000 10000 1000 100 50 20 10 5 4 3 2 1)
Tname=(baseline 10ms 1ms 100us 50us 20us 10us 5us 4us 3us 2us 1us)
trial=10

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
