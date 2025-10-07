#!/bin/bash 

# Clear previous output file if it exists
> execution_time.txt

# Function to run and time the program
run_and_time() {
    optimization_name=$1
    executable=$2

    real_time=$( { time ./$executable; } 2>&1 | grep real | awk '{print $2}' )
    echo "$optimization_name : $real_time"
    echo "$optimization_name : $real_time" >> execution_time.txt
}

# Compile the baseline implementation and run the baseline implemenation using run_and_time
gcc -O0 matrix_multiplication.c -o matrix_multiplication
run_and_time "Baseline (no optimization)" "matrix_multiplication"

# Compile with loop interchange optimization and measure time
gcc -O2 -floop-interchange matrix_multiplication.c -o matrix_multiplication_li
run_and_time "loop interchange" "matrix_multiplication_li"

# Compile with loop tiling optimization and measure time
gcc -O3 -floop-block matrix_multiplication.c -o matrix_multiplication_lt
run_and_time "loop tiling" "matrix_multiplication_lt"

# Compile with loop vectorization and measure time
gcc -O3 -ftree-vectorize matrix_multiplication.c -o matrix_multiplication_lv
run_and_time "loop vectorization" "matrix_multiplication_lv"

# Compile with OpenMP parallelization and measure time
gcc -O3 -fopenmp matrix_multiplication.c -o matrix_multiplication_mp
run_and_time "OpenMP parallelization" "matrix_multiplication_mp"



