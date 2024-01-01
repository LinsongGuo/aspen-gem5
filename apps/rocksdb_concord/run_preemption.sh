#!/bin/bash

bench=$1
system=$2

if [ "$system" == "signal" ]; then
    mechs=(signal)
    cmds=(./rocksdb_server)
elif [ "$system" == "uintr" ]; then
    mechs=(uintr)
    cmds=(./rocksdb_server)
elif [ "$system" == "concord" ]; then
    mechs=(concord_base concord concord_f concord_fl)
    cmds=(./rocksdb_server ./rocksdb_server_concord ./rocksdb_server_concord_f ./rocksdb_server_concord_fl)
else
    echo "Invalid system"
fi

Tus=(100000000 10000 5000 1000 500 200 100 50 20 15 10 7 5 4 3 2 1)
Tname=(baseline 10ms 5ms 1ms 500us 200us 100us 50us 20us 15us 10us 7us 5us 4us 3us 2us 1us)

folder="results/preemption/$bench"
if [ ! -d "$folder" ]; then
    mkdir $folder
fi 

for ((k=0; k<${#mechs[@]}; k++)); do 
    mech=${mechs[k]}
    cmd=${cmds[k]}

    folder="results/preemption/$bench/$mech"
    if [ ! -d "$folder" ]; then
        mkdir $folder
    fi 

    for ((i=0; i<${#Tus[@]}; i++)); do 
        tus=${Tus[i]}
        tname=${Tname[i]}
        echo $system, $tname
        filepath="$folder/$tname"
        
        export TIMESLICE=$tus 
        sudo rm -rf /tmp/my_db/
        sudo -E  $cmd ../../server.config 5000 >$filepath
    done

done