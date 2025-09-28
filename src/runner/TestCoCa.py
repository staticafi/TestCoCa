#!/usr/bin/env python3
import re
import subprocess
import sys
import os
import time
import shutil
import zipfile


class Config:
    def __init__(self, self_dir):
        self.self_dir = self_dir
        self.input_file = None
        self.output_dir = os.getcwd()
        self.test_suite = None
        self.testcomp = False
        self.use_m32 = False
        self.silent = False
        self.skip_building = False
        self.skip_testing = False
        self.options = []
        self.goal_options = []
        self.test_options = []
        self.ll_file = None
        self.instrumented_ll = None
        self.target_file = None


def execute_command(command, timeout=None):
    try:
        result = subprocess.run(command, timeout=timeout)
        return result.returncode == 0
    except Exception as e:
        print(f"Command failed: {' '.join(command)} - {str(e)}")
        return False


def transform_goals(input_file):
    pattern = re.compile(r'^Goal_(\d+):;$')

    try:
        with open(input_file, 'r') as f:
            content = f.readlines()

        label_count = 0
        new_content = []
        for line in content:
            stripped = line.strip()
            if pattern.match(stripped):
                new_line = f"__testcoca_process_goal({label_count});\n"
                new_content.append(new_line)
                label_count += 1
            else:
                new_content.append(line)

        new_content.insert(0, f"const int __testcoca_goal_count = {label_count};\n")

        with open(input_file, 'w') as f:
            f.writelines(new_content)

        return True
    except Exception as e:
        print(f"Error processing {input_file}: {str(e)}")
        return False


def process_llvm_ir_goals(file):
    with open(file, 'r') as f:
        content = f.read()

    call_pattern = r'(.*call.*@__testcoca_process_goal\(i32 noundef )(\d+)(\).*)'
    total_calls = len(re.findall(call_pattern, content))

    print(f"Found {total_calls} calls to __testcoca_process_goal")

    start_id = 0
    def replace_callback(match):
        nonlocal start_id
        prefix, old_id, suffix = match.groups()
        new_call = f"{prefix}{start_id}{suffix}"
        start_id += 1
        return new_call

    updated_content = re.sub(call_pattern, replace_callback, content)

    updated_content = re.sub(
        r'@__testcoca_goal_count = .* i32 \d+, align 4',
        f'@__testcoca_goal_count = dso_local constant i32 {total_calls}, align 4',
        updated_content
    )

    with open(file, 'w') as f:
        f.write(updated_content)


def instrument(config):
    cmd = ["clang-18"] + (["-m32"] if config.use_m32 else []) + [
        "-O0", "-S", "-emit-llvm",
        "-Wno-everything", "-fbracket-depth=1024",
        config.input_file, "-o", config.ll_file
    ]

    if not config.silent: print("Compiling C to LLVM...")
    if not execute_command(cmd):
        print(f"Compilation failed: {config.input_file}")
        return False

    cmd = [os.path.join(config.self_dir, "tools", "@INSTRUMENTER_FILE@")] + config.goal_options + [
        "--input", config.ll_file, "--output", config.instrumented_ll
    ]

    if not config.silent: print("Instrumenting LLVM...")
    if not execute_command(cmd):
        print(f"Instrumentation failed: {config.ll_file}")
        return False
    return True


def instrument_testcomp(config):
    input_filename = os.path.basename(config.input_file)
    input_name, input_ext = os.path.splitext(input_filename)
    temp_input_path = os.path.join(config.output_dir, f"{input_name}_tidy.c")

    with open(config.input_file, 'r') as src, open(temp_input_path, 'w') as dst:
        dst.write(src.read())

    clang_tidy_cmd = ["clang-tidy-18",
                      "--checks=-*,readability-braces-around-statements",
                      "--fix-errors",
                      temp_input_path
    ]

    if not config.silent:
        print("Running clang-tidy...")

    subprocess.run(clang_tidy_cmd, capture_output=True, text=True)

    cmd = [os.path.join(config.self_dir, "label-adder"),
            "--labels-branching-only",
            "--labels-switch-only",
            "--labels-ternary-only",
            temp_input_path
    ]

    if not config.silent: print("Adding labels...")

    result = subprocess.run(cmd, capture_output=True, text=True)

    inst_c_file = config.instrumented_ll.replace('.ll', '.c')
    with open(inst_c_file, 'w') as f:
        f.write(result.stdout)

    transform_goals(inst_c_file)

    if not config.silent: print("Transforming labes to function calls...")

    compile_cmd = ["clang-18"] + (["-m32"] if config.use_m32 else []) + [
        "-O0", "-S", "-emit-llvm",
        "-Wno-everything", "-fbracket-depth=1024",
        inst_c_file, "-o", config.ll_file
    ]

    if not config.silent: print("Compiling C to LLVM...")

    if not execute_command(compile_cmd):
        print(f"Failed to compile instrumented C code to LLVM: {inst_c_file}")
        return False

    process_llvm_ir_goals(config.ll_file)

    cmd = [os.path.join(config.self_dir, "tools", "@INSTRUMENTER_FILE@")] + [
        "--input", config.ll_file, "--output", config.instrumented_ll
    ]

    if not config.silent: print("Instrumenting LLVM...")

    if not execute_command(cmd):
        print(f"Instrumentation failed: {config.ll_file}")
        return False

    return True


def build(config):
    if not config.testcomp:
        if not instrument(config):
            return False
    else:
        if not instrument_testcomp(config):
            return False

    target_libs = [
                      os.path.join(config.self_dir, "lib32" if config.use_m32 else "lib", lib)
                      for lib in @TARGET_LIBRARIES_FILES_LIST@
    ]

    cmd = ["clang++-18"] + (["-m32"] if config.use_m32 else []) + [
        "-O3", config.instrumented_ll
    ] + "@TARGET_NEEDED_COMPILATION_FLAGS@".split() + target_libs + [
              "-o", config.target_file
          ]

    if not config.silent: print("Linking executable...")
    if not execute_command(cmd):
        print(f"Linking failed: {config.input_file}")
        return False

    if not config.silent: print("Build completed successfully")
    return True


def run_tests(config):
    if not os.path.exists(config.target_file):
        old_target = os.path.join(os.path.dirname(config.input_file), os.path.basename(config.target_file))
        if not os.path.exists(old_target):
            print(f"Target executable not found: {config.target_file}")
            return False
        target = old_target
    else:
        target = config.target_file

    cmd = ([
               os.path.join(config.self_dir, "tools", "@DRIVER_FILE@"),
               "--path_to_target", target,
               "--test_dir", config.test_suite,
               "--output_dir", config.output_dir,
           ]
           + (["--testcomp"] if config.testcomp else [])
           + config.test_options + config.options)

    if not config.silent: print("Running tests...")
    return execute_command(cmd)


def parse_coverage_goal(goal_file):
    if not goal_file:
        print("Using default branch coverage")
        return ["--inst_br"], ["--goal", "coverage"]

    try:
        with open(goal_file, 'r') as f:
            content = f.read().strip()
    except Exception as e:
        print(f"Error reading goal file: {str(e)}")
        return [], []

    # taken from https://gitlab.com/sosy-lab/software/test-suite-validator/-/blob/main/suite_validation/__init__.py
    pattern = r"COVER\s*\(\s*init\s*\(\s*main\s*\(\s*\)\s*\)\s*,\s*FQL\s*\(COVER\s+EDGES\s*\((.*)\)\s*\)\s*\)"
    match = re.search(pattern, content)
    if not match:
        print("Invalid coverage goal specification")
        return [], []

    goal_type = match.group(1).strip()
    if goal_type.startswith("@CALL"):
        err_func = goal_type[6:-1]
        return ["--inst_err", err_func], ["--goal", "call"]
    elif goal_type == "@DECISIONEDGE":
        return ["--inst_br"], ["--goal", "coverage"]

    print(f"Unsupported coverage type: {goal_type}")
    return [], []


def prepare_test_suite(test_suite, output_dir):
    if os.path.isfile(test_suite) and zipfile.is_zipfile(test_suite):
        extract_dir = os.path.join(output_dir, os.path.splitext(os.path.basename(test_suite))[0])
        if os.path.exists(extract_dir):
            shutil.rmtree(extract_dir)
        os.makedirs(extract_dir)

        try:
            with zipfile.ZipFile(test_suite, 'r') as zf:
                zf.extractall(extract_dir)
            return extract_dir
        except Exception as e:
            print(f"Error extracting test suite: {str(e)}")
            return None
    return test_suite


def print_help(self_dir):
    print("TestCoCa Usage")
    print("==============")
    print("--help                 Show this help")
    print("--input_file <PATH>    Source C file to build and test")
    print("--test_suite <PATH>    Directory containing tests")
    print("--output_dir <PATH>    Directory for results (default: current dir)")
    print("--goal <PATH>          .prp file for coverage goal (default: branch)")
    print("--testcomp             Use TestCov's label-adder to calculate branch coverage")
    print("--skip_building        Skip building step")
    print("--skip_testing         Skip testing step")
    print("--silent_mode          Suppress output messages")
    print("--m32                  Compile for 32-bit architecture")


def main():
    start_time = time.time()
    self_dir = os.path.dirname(os.path.abspath(__file__))
    config = Config(self_dir)
    goal_file = None

    i = 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg == "--help":
            print_help(self_dir)
            return 0
        elif arg == "--version":
            # TODO
            return 0
        elif arg == "--silent_mode":
            config.silent = True
        elif arg == "--input_file" and i + 1 < len(sys.argv):
            config.input_file = os.path.abspath(sys.argv[i + 1])
            i += 1
        elif arg == "--test_suite" and i + 1 < len(sys.argv):
            config.test_suite = os.path.abspath(sys.argv[i + 1])
            i += 1
        elif arg == "--goal" and i + 1 < len(sys.argv):
            goal_file = os.path.abspath(sys.argv[i + 1])
            i += 1
        elif arg == "--output_dir" and i + 1 < len(sys.argv):
            config.output_dir = os.path.abspath(sys.argv[i + 1])
            os.makedirs(config.output_dir, exist_ok=True)
            i += 1
        elif arg == "--skip_building":
            config.skip_building = True
        elif arg == "--skip_testing":
            config.skip_testing = True
        elif arg == "--testcomp":
            config.testcomp = True
        elif arg == "--m32":
            config.use_m32 = True
        else:
            config.options.append(arg)
        i += 1

    if not config.input_file or not os.path.isfile(config.input_file):
        print("Missing or invalid input file")
        return 1

    base = os.path.splitext(os.path.basename(config.input_file))[0]
    config.ll_file = os.path.join(config.output_dir, base + ".ll")
    config.instrumented_ll = os.path.join(config.output_dir, base + "_instrumented.ll")
    config.target_file = os.path.join(config.output_dir, base + "_target")

    config.goal_options, config.test_options = parse_coverage_goal(goal_file)
    if not config.goal_options or not config.test_options:
        print("Invalid coverage goal configuration")
        return 1

    os.makedirs(config.output_dir, exist_ok=True)

    if not config.skip_building:
        if not config.silent: print("Starting build process...")
        if not build(config):
            print("Build failed")
            return 1

    if not config.skip_testing:
        if not config.test_suite or not os.path.exists(config.test_suite):
            print(config.test_suite)
            print("Missing or invalid test suite")
            return 1

        prepared_test_suite = prepare_test_suite(config.test_suite, config.output_dir)
        if not prepared_test_suite:
            return 1
        config.test_suite = prepared_test_suite

        if not config.silent: print(f"Using test suite: {config.test_suite}")
        if not run_tests(config):
            print("Testing failed")
            return 1

    if not config.silent:
        print(f"Completed successfully in {time.time() - start_time:.2f} seconds")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\nExecution interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {str(e)}")
        sys.exit(1)
