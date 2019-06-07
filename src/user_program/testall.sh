#!/usr/bin/env bash

MASTER_METHOD=$1
SLAVE_METHOD=$2
MASTER_LOG='./master_log_mm'
SLAVE_LOG='./slave_log_mm'

echo "Master: $MASTER_METHOD, Slave: $SLAVE_METHOD" > $MASTER_LOG
echo "-----------------------------" >> $MASTER_LOG

echo "Master: $MASTER_METHOD, Slave: $SLAVE_METHOD" > $SLAVE_LOG
echo "-----------------------------" >> $SLAVE_LOG

for input_file in ../data/*; do
	echo $input_file >> $MASTER_LOG
	echo $input_file >> $SLAVE_LOG
	echo "-----------------------------" >> $MASTER_LOG
	echo "-----------------------------" >> $SLAVE_LOG

	for i in {1..30}; do
		echo $i
		sudo ./master $input_file $MASTER_METHOD >> $MASTER_LOG &
		sudo ./slave ../test_output $SLAVE_METHOD 127.0.0.1 >> $SLAVE_LOG
		sleep .2		
	done

	echo "-----------------------------" >> $MASTER_LOG
	echo "-----------------------------" >> $SLAVE_LOG
done
