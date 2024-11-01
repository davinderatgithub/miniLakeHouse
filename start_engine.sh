#!/bin/bash
for port in "$@"
do
    ./build/engine_main $port &
done
wait
