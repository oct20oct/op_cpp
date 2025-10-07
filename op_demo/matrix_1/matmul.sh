#!/bin/bash

# === Compiler and Flags ===
CXX=g++
CXXFLAGS=-Ofast
EXTRA_FLAGS="-DLITE_ARRAY_NO_HINT -DNDEBUG"

# === OpenBLAS Paths ===
#OPENBLAS_INCLUDE="$(brew --prefix openblas)/include/"
#OPENBLAS_LIB="$(brew --prefix openblas)/lib"
OPENBLAS_INCLUDE="/usr/include/openblas"
OPENBLAS_LIB="/usr/lib"

# === Output and Source Files ===
SOURCE_FILE="matmul.cpp"
OUTPUT_FILE="matmul"

# Complete and add the correct flags ===
# 
# 1. Add profiling support
# 2. Add OpenBLAS paths
# 4. Link against OpenBLAS

$CXX $CXXFLAGS $EXTRA_FLAGS -I$OPENBLAS_INCLUDE -L$OPENBLAS_LIB $SOURCE_FILE -o $OUTPUT_FILE -lopenblas -pg

# Execute the program and output the results to data.txt
./matmul > data.txt

# === Generate the profiling report ===
# 
# - Use `gprof` to analyze the program's profiling data
# - The compiled executable is `./matmul`
# - The profiling data file is `gmon.out`
# - Redirect the output to a file named `profile_report.txt`
gprof $OUTPUT_FILE gmon.out > profile_report.txt
