#!/bin/bash

mac="$1"

ip=$(ip neigh | grep -v FAIL | grep ${mac} | cut -d' ' -f 1 | grep -v :)

if [[ -z $ip ]]; then
    echo
else
    printf "$ip\n"
fi
