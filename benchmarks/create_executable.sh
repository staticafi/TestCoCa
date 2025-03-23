#!/bin/bash

# Initialize variables
input_file=""
output_bin=""
output_dir="."

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --input)
            input_file="$2"
            shift 2
            ;;
        --output)
            output_bin="$2"
            shift 2
            ;;
        --output_dir)
            output_dir="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 --input FILE --output EXECUTABLE [--output_dir DIR] [--help]"
            echo "Compile and instrument a C program to be used by the qm2c tool"
            exit 1;
            ;;
        *)
            echo "Error: Unknown option $1"
            exit 1
            ;;
    esac
done

# Validate required parameters
if [[ -z "$input_file" ]]; then
    echo "Error: --input parameter is required"
    exit 1
fi

if [[ -z "$output_bin" ]]; then
    echo "Error: --output parameter is required"
    exit 1
fi

# Create output directory if needed
if [[ "$output_dir" != "." && ! -d "$output_dir" ]]; then
    mkdir -p "$output_dir" || {
        echo "Error: Failed to create output directory: $output_dir"
        exit 1
    }
fi

# Generate filenames
base_name=$(basename "${input_file%.c}")
ll_file="${output_dir}/${base_name}.ll"
instr_ll="${output_dir}/${base_name}_instr.ll"
final_bin="${output_dir}/${output_bin}"

# Generate LLVM IR
echo "Generating LLVM IR from: $input_file"
clang -S -emit-llvm -o "$ll_file" "$input_file" || {
    echo "Error: Failed to generate LLVM IR"
    exit 1
}

# Instrument IR
echo "Instrumenting LLVM IR: $ll_file"
../build/src/instrumenter/instrumenter \
    --input "$ll_file" \
    --output "$instr_ll" || {
    echo "Error: Instrumentation failed"
    exit 1
}

# Build and link
echo "Compiling instrumented code to: $final_bin"
clang++ -O0 "$instr_ll" \
    ../build/src/target/libtarget.a \
    ../build/src/connection/libconnection.a \
    ../build/src/iomodels/libiomodels.a \
    ../build/src/utility/libutility.a \
    -lstdc++ -lm -lpthread -lrt -ldl \
    -o "$final_bin" || {
    echo "Error: Compilation failed"
    exit 1
}

echo "Successfully built:"
echo " - LLVM IR:          $ll_file"
echo " - Instrumented IR:   $instr_ll"
echo " - Final executable:  $final_bin"
