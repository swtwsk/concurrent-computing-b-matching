#!/bin/bash

liczbaParametrow=$#

if [ $liczbaParametrow -eq 4 ]
then
	prog=$1
	max_thread_count=$2
	input_file=$3
	b_limit=$4
	nazwa=$(basename $input_file .txt)
	failed=0

	for thread in `seq 1 $max_thread_count`
	do
		./$prog $thread $input_file $b_limit > ${nazwa}${thread}.out
		if [ $thread -gt 1 ]
		then
			last_thread=$[thread - 1]
			diff=$(diff ${nazwa}${last_thread}.out ${nazwa}${thread}.out)
			if [ "$diff" != "" ]
			then
				echo "test failed in $thread"
				failed=$[failed + 1]
			else
				echo "test $thread OK"
			fi
		else
			echo "test $thread OK"
		fi
	done

	if [ $failed -eq 0 ]
	then
		for file_name in `seq 1 $max_thread_count`
		do
			rm ${nazwa}${file_name}.out
		done
	fi
else
	echo "USAGE: ./test.sh program-name thread-count input-file limit-b"
fi
