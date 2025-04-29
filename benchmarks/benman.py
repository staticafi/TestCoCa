import glob
import os
import sys
import json
import shutil
import argparse
import subprocess


def ASSUMPTION(cond, msg="Unknown."):
    if not cond:
        raise Exception(msg)


def quote(path : str) -> str:
    return '"' + path + '"'


class Benchmark:
    def __init__(self, pathname : str, runner_script : str, verbose : bool) -> None:
        self.runner_script = runner_script
        self.verbose = verbose

        self.python_binary = sys.executable

        self.work_dir = os.path.dirname(pathname).replace("\\", "/")
        self.fname = os.path.basename(pathname)
        self.name = os.path.splitext(self.fname)[0]

        self.src_file = os.path.join(self.work_dir, self.fname)
        self.config_file = os.path.join(self.work_dir, self.name + ".json")
        if os.path.isdir(os.path.join(self.work_dir, "test-suite")):
            self.test_suite = os.path.join(self.work_dir, "test-suite")
        else:
            self.test_suite = os.path.join(self.work_dir, "test-suite.zip")

        with open(self.config_file, "rb") as fp:
            self.config = json.load(fp)
        ASSUMPTION(all(x in self.config for x in ["args", "results"]), "Cannot find 'args' or 'results' in the benchmark's JSON file.")
        ASSUMPTION(all(x in self.config["args"] for x in [
            "max_exec_milliseconds",
            "max_exec_megabytes",
            ]), "Benchmark's JSON file does not contain all required options for running the tool.")

        self.instrumented_bin = self.name + "_instr.out"

        self.dir_stack = []

    def pushd(self, folder : str) -> None:
        self.dir_stack.append(os.getcwd())
        os.chdir(folder)

    def popd(self) -> str:
        ASSUMPTION(len(self.dir_stack) > 0, "Cannot pop from the empty stack of directories.")
        os.chdir(self.dir_stack[-1])
        del self.dir_stack[-1]

    def log(self, message : str, brief_message: str = None) -> None:
        if self.verbose:
            print(">>> " + message)
            sys.stdout.flush()
        elif brief_message is not None:
            print(brief_message, end="")
            sys.stdout.flush()

    def _erase_file_if_exists(self, pathname : str) -> None:
        if os.path.exists(pathname):
            self.log("remove " + pathname)
            os.remove(pathname)

    def _erase_dir_if_exists(self, pathname : str) -> None:
        if os.path.exists(pathname):
            self.log("rmtree " + pathname)
            shutil.rmtree(pathname)

    def _compute_output_dir(self, benchmarks_root_dir : str, output_root_dir : str):
        return os.path.dirname(os.path.join(output_root_dir, os.path.relpath(self.src_file, benchmarks_root_dir)))

    def _execute(self, cmdline : str, output_dir : str) -> None:
        cmd = [x for x in cmdline if len(x) > 0]
        self.log(" ".join(cmd))
        subprocess.run(cmd)

    def _execute_and_check_output(self, cmdline : str, desired_output : str, work_dir : str = None) -> None:
        self._execute(cmdline, os.path.dirname(desired_output) if work_dir is None else work_dir)
        ASSUMPTION(os.path.isdir(desired_output), "_execute_and_check_output(): the output is missing: " + desired_output)

    @staticmethod
    def _add_error_message(text: str, errors: list, properties: list):
        errors.append(("In " + "/".join(properties) + ": " if len(properties) > 0 else "") + text)

    @staticmethod
    def _epsilon_for_property(properties: list, expected: int|float):
        if len(properties) == 0: return None
        if properties[-1] == "num_executions": return 5.0 if expected >= 100 else 50.0 
        return None

    @staticmethod
    def _check_outcomes(obtained, expected, errors: list, properties = []) -> bool:
        if type(expected) is dict:
            if type(obtained) is not dict:
                Benchmark._add_error_message("Mismatch in JSON structure. Expected dictionary.", errors, properties)
                return False
            if len(obtained) > len(expected) and "output_statistics" in properties:
                Benchmark._add_error_message("Too many keys in obtained dictionary. "
                                             "Obtained: " + str(len(obtained)) + ", Expected: " + str(len(expected)), errors, properties)
            result = True
            for key in expected:
                if key not in obtained:
                    Benchmark._add_error_message("Missing property: " + key, errors, properties)
                    return False
                r = Benchmark._check_outcomes(obtained[key], expected[key], errors, properties + [key])
                result = result and r
            return result
        elif type(expected) is list:
            if type(obtained) is not list:
                Benchmark._add_error_message("Mismatch in JSON structure. Expected list.", errors, properties)
                return False
            if len(obtained) != len(expected):
                Benchmark._add_error_message("Different list size.", errors, properties)
                return False
            result = True
            for i in range(min(len(obtained), len(expected))):
                r = Benchmark._check_outcomes(obtained[i], expected[i], errors, properties)
                result = result and r
            return result
        elif type(expected) in [int, float]:
            if type(obtained) not in [int, float]:
                Benchmark._add_error_message("Mismatch in JSON structure. Expected int or float.", errors, properties)
                return False
            epsilon = Benchmark._epsilon_for_property(properties, expected)
            if epsilon is None:
                if obtained != expected:
                    Benchmark._add_error_message("Expected " + str(expected) + ", obtained " + str(obtained), errors, properties)
                    return False
            else:
                percentage = (100.0 * obtained) / expected if expected > 0 else 100.0 * obtained + 100.0
                error = percentage - 100.0
                if abs(error) > epsilon:
                    Benchmark._add_error_message("Expected " + str(expected) + ", obtained " + str(obtained) + " [error: " + ("%.2f" % error) + "%]", errors, properties)
                    return False
            return True
        elif type(expected) is str:
            if type(obtained) is not str:
                Benchmark._add_error_message("Mismatch in JSON structure. Expected string.", errors, properties)
                return False
            if obtained != expected:
                Benchmark._add_error_message("Expected " + expected + ", obtained " + obtained, errors, properties)
                return False
            return True
        else:
            Benchmark._add_error_message("Unexpected JSON content [type: " + str(type(expected)) + "].", errors, properties)
            return False

    def build(self, benchmarks_root_dir : str, output_root_dir : str) -> None:
        self.log("===")
        self.log("=== Building: " + self.src_file, "building: " + os.path.relpath(self.src_file, os.path.dirname(self.work_dir)) + " ... ")
        self.log("===")

        output_dir = self._compute_output_dir(benchmarks_root_dir, output_root_dir)
        self.log("makedirs " + output_dir)
        os.makedirs(output_dir, exist_ok=True)

        self._execute_and_check_output(
            [
                benman.runner_script,
                "--skip_testing",
                "--input_file", self.src_file,
                "--output_dir", output_dir,
            ] + (["--m32"] if "m32" in self.config["args"] and self.config["args"]["m32"] is True else []) +
            ((["--goal"] + [self.config["args"]["goal"]]) if "goal" in self.config["args"] else []),
            output_dir
            )

        self.log("Done", "Done\n")

    def test(self, benchmarks_root_dir : str, output_root_dir : str) -> bool:
        self.log("===")
        self.log("=== testing: " + self.src_file)
        self.log("===")

        output_dir = self._compute_output_dir(benchmarks_root_dir, output_root_dir)

        self.log("makedirs " + output_dir)
        os.makedirs(output_dir, exist_ok=True)

        self._execute(
            [
                benman.runner_script,
                "--skip_building",
                "--input_file", self.src_file,
                "--test_suite", self.test_suite,
                "--output_dir", output_dir,
            ] +
            ((["--goal"] + [self.config["args"]["goal"]]) if "goal" in self.config["args"] else []),
            output_dir
            )

        try:
            outcomes_pathname = os.path.join(output_dir, "result.json")
            with open(outcomes_pathname, "rb") as fp:
                outcomes = json.load(fp)
                expected_result = float (self.config["results"]["coverage"])
                actual_result = float (outcomes["coverage"])

                if expected_result == actual_result:
                    return True

                if ("coverage_map" in outcomes and "coverage_map" in self.config["results"]):
                    expected_coverage = self.config["results"]["coverage_map"]
                    actual_coverage = outcomes["coverage_map"]

                    if expected_coverage == actual_coverage:
                        return True

                return False

        except Exception as e:
            self.log("FAILURE due to an EXCEPTION: " + str(e), "EXCEPTION[" + str(e) + "]\n")
            return False

        print("FAILED", flush=True)
        return False

    def clear(self, benchmarks_root_dir : str, output_root_dir : str) -> None:
        self.log("===")
        self.log("=== Clearing: " + self.src_file, "clearing: " + os.path.relpath(self.src_file, os.path.dirname(self.work_dir)) + " ... ")
        self.log("===")
        self._erase_dir_if_exists(self._compute_output_dir(benchmarks_root_dir, output_root_dir))
        self.log("Done", "Done\n")


class Benman:
    def __init__(self) -> None:
        parser = argparse.ArgumentParser(description="Builds the target for the benchmark(s) or test the benchmark(s).")
        parser.add_argument("--clear", action='store_true', help="Clears the build files and outputs of the input benchmark(s).")
        parser.add_argument("--build", action='store_true', help="Builds the input benchmark(s).")
        parser.add_argument("--test", action='store_true', help="Test the coverage of input benchmark(s)")
        parser.add_argument("--input", help="Benchmark(s) to be processed. Possible values: "
                                           "all, cover_branches, cover_error, cover_*/...")
        parser.add_argument("--verbose", action='store_true', help="Enables the verbose mode.")
        self.args = parser.parse_args()

        self.python_binary = '"' + sys.executable + '"'
        self.benchmarks_dir = os.getcwd()
        self.output_dir = os.path.normpath(os.path.join(self.benchmarks_dir, "..", "output", "benchmarks"))

        self.runner_script = os.path.join(self.benchmarks_dir, "..", "TestCoCa.py")
        ASSUMPTION(os.path.isfile(self.runner_script), "The runner script not found. Build and install the project first.")

    def collect_benchmarks(self, name : str) -> list[str]:
        def complete_and_check_benchmark_path(benchmark_path : str) -> str:
            benchmark_dir = os.path.join(self.benchmarks_dir, benchmark_path)
            benchmark_name = os.path.basename(benchmark_dir)

            ASSUMPTION(os.path.isdir(benchmark_dir),
                       f"Benchmark directory not found: {benchmark_dir}")

            c_file_path = os.path.join(benchmark_dir, f"{benchmark_name}.c")
            ASSUMPTION(os.path.isfile(c_file_path),
                       f"Missing main C file: {c_file_path}")

            json_path = os.path.join(benchmark_dir, f"{benchmark_name}.json")
            ASSUMPTION(os.path.isfile(json_path),
                       f"Missing JSON configuration: {json_path}")

            if os.path.exists(os.path.join(benchmark_dir, "test-suite.zip")):
                test_suite_dir = os.path.join(benchmark_dir, "test-suite.zip")
            else:
                test_suite_dir = os.path.join(benchmark_dir, "test-suite")
            ASSUMPTION(os.path.exists(test_suite_dir),
                       f"Missing test_suite directory in {benchmark_dir}")

            if os.path.isdir(test_suite_dir):
                test_pattern = os.path.join(test_suite_dir, f"*test_*.xml")
                test_files = glob.glob(test_pattern)
                ASSUMPTION(len(test_files) >= 1,
                           f"Expected at least 1 test files matching *test_*.xml, found {len(test_files)}")

                metadata_path = os.path.join(test_suite_dir, "metadata.xml")
                ASSUMPTION(os.path.isfile(metadata_path),
                           f"Missing metadata.xml in test suite")

            return c_file_path

        def search_for_benchmarks(folder : str) -> list:
            benchmarks = []
            pathname = os.path.join(self.benchmarks_dir, folder)
            if os.path.isdir(pathname):
                for name in os.listdir(pathname):
                    if os.path.isdir(os.path.join(folder, name)):
                        benchmarks.append(complete_and_check_benchmark_path(os.path.join(folder, name)))
            return benchmarks

        kinds = ["cover_branches", "cover_error"]
        benchmarks = []
        if name == "all":
            for kind in kinds:
                benchmarks += search_for_benchmarks(kind)
        elif name in kinds:
            benchmarks += search_for_benchmarks(name)
        else:
            benchmarks.append(complete_and_check_benchmark_path(name))
        return sorted(benchmarks)

    def build(self, name : str) -> bool:
        for pathname in self.collect_benchmarks(name):
            benchmark = Benchmark(pathname, self.runner_script, self.args.verbose)
            benchmark.build(self.benchmarks_dir, self.output_dir)
        return True

    def test(self, name : str):
        num_failures = 0
        benchmark_paths = self.collect_benchmarks(name)
        for pathname in benchmark_paths:
            benchmark = Benchmark(pathname, self.runner_script, self.args.verbose)
            if not benchmark.test(self.benchmarks_dir, self.output_dir):
                print("FAILED")
                num_failures += 1
        if num_failures > 0:
            print("FAILURE[" + str(num_failures) + "/" + str(len(benchmark_paths)) + "]")
            return False
        else:
            print("SUCCESS")
            return True

    def clear(self, name : str) -> None:
        for pathname in self.collect_benchmarks(name):
            benchmark = Benchmark(pathname, self.runner_script, self.args.verbose)
            benchmark.clear(self.benchmarks_dir, self.output_dir)

    def run(self) -> bool:
        if self.args.clear:
            if self.clear(self.args.input) is False:
                return False
        if self.args.build:
            if self.build(self.args.input) is False:
                return False
        if self.args.test:
            if self.test(self.args.input) is False:
                return False
        return True

if __name__ == '__main__':
    exit_code = 0
    try:
        benman = Benman()
        if benman.run() is False:
            exit_code = 1
    except Exception as e:
        exit_code = 1
        print("ERROR: " + str(e))
    exit(exit_code)
