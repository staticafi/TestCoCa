#!/usr/bin/env python3
import subprocess
import sys
import os
import time
import shutil
import zipfile
from pathlib import Path


def _execute(command_and_args, timeout_ = None):
    cmd = [x for x in command_and_args if len(x) > 0]
    return subprocess.run(cmd, timeout=timeout_)


def  benchmark_file_name(input_file):
    return os.path.basename(input_file)


def  benchmark_name(input_file):
    return os.path.splitext(benchmark_file_name(input_file))[0]


def  benchmark_ll_name(input_file):
    return benchmark_name(input_file) + ".ll"


def  benchmark_instrumented_ll_name(input_file):
    return benchmark_name(input_file) + "_instrumented.ll"


def  benchmark_target_name(input_file):
    return benchmark_name(input_file) + "_target"

def build(self_dir, input_file, output_dir, options, use_m32, silent_mode):
    ll_file = os.path.join(output_dir, benchmark_ll_name(input_file))

    if silent_mode is False: print("\"build_times\": {", flush=True)
    if silent_mode is False: print("    \"Compiling[C->LLVM]\": ", end='', flush=True)
    t0 = time.time()
    if _execute(
            [ "clang" ] +
                (["-m32"] if use_m32 is True else []) +
                [ "-O0", "-g", "-S", "-emit-llvm", "-Wno-everything", "-fbracket-depth=1024", input_file, "-o", ll_file],
            None).returncode:
        raise Exception("Compilation[C->LLVM] has failed: " + input_file)
    t1 = time.time()
    if silent_mode is False: print("%.2f," % (t1 - t0), flush=True)

    instrumented_ll_file = os.path.join(output_dir, benchmark_instrumented_ll_name(input_file))
    if silent_mode is False: print("    \"Instrumenting\": ", end='', flush=True)
    t0 = time.time()
    if _execute(
            [ os.path.join(self_dir, "tools", "@INSTRUMENTER_FILE@") ] +
                options +
                ["--input", ll_file, "--output", instrumented_ll_file],
            None).returncode:
        raise Exception("Instrumentation has failed: " + ll_file)
    t1 = time.time()
    if silent_mode is False: print("%.2f," % (t1 - t0), flush=True)

    target_libraries = list(map( # type: ignore
        lambda lib_name: os.path.join(self_dir, "lib32" if use_m32 is True else "lib", lib_name).replace("\\", "/"),
        @TARGET_LIBRARIES_FILES_LIST@ # type: ignore
        ))
    target_file = os.path.join(output_dir, benchmark_target_name(input_file))

    if silent_mode is False: print("    \"Linking\": ", end='', flush=True)
    t0 = time.time()
    if _execute(
            [ "clang++" ] +
                (["-m32"] if use_m32 is True else []) +
                [ "-O3", instrumented_ll_file ] +
                "@TARGET_NEEDED_COMPILATION_FLAGS@".split() +
                target_libraries +
                [ "-o", target_file ],
            None).returncode:
        raise Exception("Linking has failed: " + input_file)
    t1 = time.time()
    if silent_mode is False: print("%.2f," % (t1 - t0), flush=True)

    if silent_mode is False: print("},", flush=True)


def test(self_dir, input_file, test_suite, output_dir, options, start_time, silent_mode):
    target = os.path.join(output_dir, benchmark_target_name(input_file))
    if not os.path.isfile(target):
        target = os.path.join(os.path.dirname(input_file), benchmark_target_name(input_file))
        if not os.path.isfile(target):
            raise Exception("Cannot find the test target file: " + target)

    if _execute(
            [ os.path.join(self_dir, "tools", "@DRIVER_FILE@"),
                "--path_to_target", target ] +
                [ "--test_dir", test_suite] +
                [ "--output_dir", output_dir] +
                options,
            None).returncode:
        raise Exception("Testing has failed.")


def help(self_dir):
    print("TestCoCa usage")
    print("================")
    print("help                 Prints this help message.")
    print("input_file <PATH>    A source C file to build and test.")
    print("test_suite <PATH>      A directory where tests are located.")
    print("output_dir <PATH>    A directory under which all results and intermediate files will be saved.")
    print("                     If not specified, then the current directory is used.")
    print("skip_building        Skip building of the source C file.")
    print("skip_testing         Skip testing of the built source C file.")
    print("silent_mode          When specified, no messages will be printed.")
    print("m32                  When specified, the source C file will be compiled for")
    print("                     32-bit machine (cpu). Otherwise, 64-bit machine is assumed.")
    print("\nNext follows a listing of options of tools called from this script. When they are")
    print("passed to the script they will automatically be propagated to the corresponding tool.")

    print("\nThe options of the LLVM 'instrumenter' tool:")
    _execute([ os.path.join(self_dir, "tools", "@INSTRUMENTER_FILE@"), "--help"], None)
    print("\nThe options of the 'driver' tool:")
    _execute([ os.path.join(self_dir, "tools", "@DRIVER_FILE@"), "--help"], None)

    print("\n!!! WARNING !!!!")
    print("An analyzed program is currently *NOT* executed in an isolated environment. It is thus")
    print("*NOT* advised to use it on a C program accessing disk or any other external resource")
    print("(unless you provided the isolation, e.g. by running the analysis in a Docker container).")


def version(self_dir):
    _execute([ os.path.join(self_dir, "tools", "@DRIVER_FILE@"), "--version"], None)

def prepare_test_suite(test_suite: str, output_dir: str) -> str:
    ts_path = Path(test_suite)
    out_path = Path(output_dir)

    # Handle ZIP files
    if ts_path.is_file() and zipfile.is_zipfile(ts_path):
        extract_dir = out_path / "test-suite"

        # Clean existing directory
        if extract_dir.exists():
            shutil.rmtree(extract_dir)

        # Create and extract
        extract_dir.mkdir(parents=True, exist_ok=True)

        try:
            with zipfile.ZipFile(ts_path, 'r') as zf:
                zf.testzip()  # Validate first
                zf.extractall(extract_dir)
        except zipfile.BadZipFile as e:
            raise ValueError(f"Invalid/corrupt ZIP file: {ts_path}") from e

        return str(extract_dir.resolve())

    # Handle existing directories
    if ts_path.is_dir():
        return test_suite

    raise FileNotFoundError(f"Test suite path not found: {test_suite}")

def main():
    start_time = time.time()
    self_dir = os.path.normpath(os.path.abspath(os.path.dirname(__file__)))
    old_cwd = os.path.abspath(os.getcwd())
    input_file = None
    test_suite = None
    output_dir = old_cwd
    clear_output_dir = False
    skip_building = False
    skip_testing = False
    silent_mode = False
    use_m32 = False
    options = []
    options_instument = []
    i = 1

    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg == "--help":
            help(self_dir)
            return
        if arg == "--version":
            version(self_dir)
            return
        if arg == "--silent_mode":
            silent_mode = True
        if arg == "--input_file" and i+1 < len(sys.argv) and os.path.isfile(sys.argv[i+1]):
            input_file = os.path.normpath(os.path.abspath(sys.argv[i+1]))
            i += 1
        elif arg == "--test_suite" and i+1 < len(sys.argv) and not os.path.isfile(sys.argv[i+1]):
            test_suite = os.path.normpath(os.path.abspath(sys.argv[i+1]))
            i += 1
        elif arg == "--output_dir" and i+1 < len(sys.argv) and not os.path.isfile(sys.argv[i+1]):
            output_dir = os.path.normpath(os.path.abspath(sys.argv[i+1]))
            os.makedirs(output_dir, exist_ok=True)
            i += 1
        elif arg == "--clear_output_dir":
            clear_output_dir = True
        elif arg == "--skip_building":
            skip_building = True
        elif arg == "--skip_testing":
            skip_testing = True
        elif arg == "--m32":
            use_m32 = True
        elif arg in [ "--inst_br" ]:
            options_instument.append(arg)
        elif arg in [ "--inst_err" ]:
            options_instument.append(arg)
            options_instument.append(sys.argv[i+1])
            i += 1
        else:
            options.append(arg)
        i += 1

    if clear_output_dir is True and os.path.isdir(output_dir):
        shutil.rmtree(output_dir)

    old_cwd = os.getcwd()
    os.chdir(output_dir)
    try:
        if input_file is None:
            raise Exception("Cannot find the input file.")
        if skip_building is False:
            build(self_dir, input_file, output_dir, options_instument, use_m32, silent_mode)
        if skip_testing is False:
            if test_suite is None:
                raise Exception("Cannot find the test directory")

            try:
                test_suite = prepare_test_suite(test_suite, output_dir)
            except (ValueError, FileNotFoundError) as e:
                raise Exception(f"Test suite preparation failed: {str(e)}")

            if not Path(test_suite).exists():
                raise FileNotFoundError(f"Final test suite path invalid: {test_suite}")

            if silent_mode is False: print(f"Test suite dir: {test_suite}", flush=True)

            test(self_dir, input_file, test_suite, output_dir, options, start_time, silent_mode)

            if silent_mode is False: print(",", flush=True)
        if silent_mode is False: print("\"exit_code\": 0,", flush=True)
    except Exception as e:
        os.chdir(old_cwd)
        if silent_mode is False: print("\"error_message\": \"" + str(e) + "\"", flush=True)
        if silent_mode is False: print("\"exit_code\": 1,", flush=True)
        raise e
    finally:
        if silent_mode is False:
            print("\"total_time\": %.2f" % (time.time() - start_time), flush=True)
            print("}", flush=True)


if __name__ == "__main__":
    exit_code = 0
    try:
        main()
    except Exception as e:
        exit_code = 1
    exit(exit_code)
