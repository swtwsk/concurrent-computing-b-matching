#!/bin/bash

liczbaParametrow=$#

if [ $liczbaParametrow -eq 4 ]
then
	prog=$1
	max_thread_count=$2
	input_file=$3
	b_limit=$4

	for thread in `seq 1 $max_thread_count`
	do
		nazwa=$(basename $input_file .txt)
		./$prog $b_limit $input_file $thread > ${nazwa}${thread}.out
		if [ $thread -gt 1 ]
		then
			last_thread=$thread - 1
			diff=$(diff ${nazwa}${last_thread}.out ${nazwa}${thread}.out)
			if [ "$diff" != "" ]
			then
				echo "test failed in $thread"
			fi
		fi
	done
else
	echo "USAGE: ./test.sh program-name thread-count input-file limit-b"
fi
