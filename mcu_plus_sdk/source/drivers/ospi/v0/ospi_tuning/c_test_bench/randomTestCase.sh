#!/bin/bash

for n in {1..200};
do
	echo $n
	timeout 15 ./c_test_bench $n >> output.txt
	if [[ $? -eq 124 ]]; then
		echo "Timeout, skipping"
		continue
	fi
	sleep 5

done

sleep 30

for n in {201..400};
do
	echo $n
	timeout 15 ./c_test_bench $n >> output.txt
	if [[ $? -eq 124 ]]; then
		echo "Timeout, skipping"
		continue
	fi
	sleep 5

done

sleep 30

for n in {401..500};
do
	echo $n
	timeout 15 ./c_test_bench $n >> output.txt
	if [[ $? -eq 124 ]]; then
		echo "Timeout, skipping"
		continue
	fi
	sleep 5

done