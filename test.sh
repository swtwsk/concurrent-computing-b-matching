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

	#CREATE NEW DIRECTIORIES FOR OUTPUTS
	mkdir outs
	mkdir times

	#ITERATE OVER THREAD_COUNT
	for thread in `seq 1 $max_thread_count`
	do
		./$prog $thread $input_file $b_limit 1>outs/${nazwa}${thread}.out 2>times/${nazwa}${thread}.tim
		if [ $thread -gt 1 ]
		then
			last_thread=$[thread - 1]
			diff=$(diff outs/${nazwa}${last_thread}.out outs/${nazwa}${thread}.out)
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

	#if [ $failed -eq 0 ]
	#then
	#	rm -rf outs
	#fi
else
	echo "USAGE: ./test.sh program-name thread-count input-file limit-b"
fi
