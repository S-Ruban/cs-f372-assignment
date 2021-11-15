#! /bin/bash
# rm ./ninja
let initial_workload=1000
let choice=1
for workload_multiplier in {1..1000}; do
	let workload=$((initial_workload*workload_multiplier))
	./output.out $choice $workload # >> ./ninja
	# echo $workload
	#sleep 1
	# echo $workload
done
