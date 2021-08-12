#!/bin/bash

date

var=$(date)

if [[ $var = *"Ağu"* ]]
then
	echo OFA
fi

var2=5

while [ $var2 -lt 10 ]
do
	echo OFAv2
	sleep 1
done
