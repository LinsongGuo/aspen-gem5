#!/bin/bash
bench=$1
config=$2
echo $bench, $config
uths=(1 2 4 6 8 10 12 16 20 24 28 32 40 48 56 64)

folder="tbench_data/$bench"
if [ ! -d "$folder" ]; then
    mkdir $folder
fi 


folder="tbench_data/$bench/$config"
if [ ! -d "$folder" ]; then
    mkdir $folder
fi 

for uth in ${uths[@]}; do 
    ./tbench ../../server.config 5000 $uth > $folder/$uth
done 
