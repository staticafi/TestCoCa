#!/bin/bash

# Check if input and output parameters are provided
if [ $# -ne 2 ]; then
    echo "Usage: $0 <input.c> <output_executable>"
    exit 1
fi

input_file="$1"
output_file="$2"
ll_file="${input_file%.c}.ll"
instr_ll="${input_file%.c}.instr.ll"

# Generate LLVM IR
clang -S -emit-llvm -o "$ll_file" "$input_file" || {
    echo "Error generating LLVM IR"
    exit 1
}
echo "LLVM IR file created"

# Instrument
./build/src/instrumenter/instrumenter --input "$ll_file" --output "$instr_ll" || {
    echo "Error during instrumentation"
    exit 1
}
echo "LLVM IR file instrumented"

# Build and link
clang++ -O0 "$instr_ll" \
    ./build/src/target/libtarget.a \
    ./build/src/connection/libconnection.a \
    ./build/src/iomodels/libiomodels.a \
    ./build/src/utility/libutility.a \
    -lstdc++ -lm -lpthread -lrt -ldl \
    -o "$output_file"


echo "Successfully created executable: $output_file"
