#!/bin/bash
for port in "$@"
do
    ./engine_main $port &
done
wait
