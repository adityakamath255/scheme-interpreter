#!/bin/bash

ITERATIONS=100

echo "Benchmarking ./scheme code.scm for $ITERATIONS runs..."
echo

total_time=0

for i in $(seq 1 $ITERATIONS); do
    echo -n "Run #$i: "
    start_time=$(date +%s.%N)
    
    ./scheme --no-repl code.scm > /dev/null
    
    end_time=$(date +%s.%N)
    elapsed=$(echo "$end_time - $start_time" | bc)
    printf "%.6f seconds\n" "$elapsed"

    total_time=$(echo "$total_time + $elapsed" | bc)
done

avg_time=$(echo "scale=8; $total_time / $ITERATIONS" | bc)

echo
printf "\nAverage execution time: %.6f seconds\n" "$avg_time"

