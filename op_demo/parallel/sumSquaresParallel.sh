#!/bin/bash

# Compile the C++ program with optimization flags 
g++ -fopenmp -O3 -o sumSquaresParallel sumSquaresParallel.cpp

# Measure start time
start=$(date +%s.%N)

# Execute the program with GNU Parallel
cat data.txt | parallel -j+0 ./sumSquaresParallel {}

# Measure end time
end=$(date +%s.%N)

# Calculate and print execution time
echo "Execution time: $(echo "$end - $start" | bc) seconds"
