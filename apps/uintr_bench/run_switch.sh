#!/bin/bash

work=$1
mode=$2

path="switch_results/$work"
if [ ! -d "$path" ]; then
	mkdir $path
fi 

path="switch_results/$work/$mode"
if [ ! -d "$path" ]; then
	mkdir $path
fi 

if [ "$mode" == "baseline" ]; then
    export UINTR_TIMESLICE=1000000000
else
    export UINTR_TIMESLICE=10 
fi

trial=21
for ((i=1; i<=16; i++)); do
	path2="$path/$i"
	if [ ! -d "$path2" ]; then
		mkdir $path2
	fi 
done

for ((j=3; j<=$trial; j++)); do
	for ((i=1; i<=16; i++)); do
	    echo $work_spec $j
		work_spec="$work*$i"
		filename="$path/$i/$j"
        ./bench ../../server.config $work_spec >$filename 
    done
done
