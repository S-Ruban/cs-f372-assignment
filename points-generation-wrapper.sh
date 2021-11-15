#! /bin/bash
let initial_workload=1000
for choice in {1..2}; do
	for workload_multiplier in {1..1000}; do
		let workload=$((initial_workload*workload_multiplier))
		./output.out $choice $workload
	done
done