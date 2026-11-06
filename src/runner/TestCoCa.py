#!/usr/bin/env python3
import re
import subprocess
import sys
import os
import time
import shutil
import zipfile
import json
import signal


FINALISATION_RESERVE_SECONDS = 0.15
DRIVER_GRACE_SECONDS = 0.5


class Config:
    def __init__(self, self_dir):
        self.self_dir = self_dir
        self.input_file = None
        self.output_dir = os.getcwd()
        self.test_suite = None
        self.testcomp = False
        self.decision_coverage = False
        self.format = False
        self.use_m32 = False
        self.silent = False
        self.skip_building = False
        self.skip_testing = False
        self.options = []
        self.mode = None
        self.error_func = ""
        self.tools = {}
        self.ll_file = None
        self.instrumented_ll = None
        self.target_file = None
        self.max_exec_milliseconds = 1000
        self.deadline = None

    def remaining_seconds(self, reserve=0.0):
        if self.deadline is None:
            return None
        return self.deadline - time.monotonic() - reserve

    def command_timeout(self):
        return self.remaining_seconds(FINALISATION_RESERVE_SECONDS)


# ---------------------------------------------------------------------------
# Instrumenter pipeline model
#
# A coverage "mode" is described as data: an ordered list of stages plus the
# exact driver flag binding. Each stage is either a process-backed tool (named
# here, resolved through the tool table) or an in-process Python function. The
# executor treats both identically, so adding a mode means adding a descriptor,
# not editing dispatch logic.
# ---------------------------------------------------------------------------

class Stage:
    def __init__(self, tool=None, args=None, func=None, stdout_to=None, sets=None):
        self.tool = tool
        self.args = args or []
        self.func = func
        self.stdout_to = stdout_to
        # Mapping of context key -> context key applied after the stage runs,
        # used to feed a rewritten source into the following compile stage.
        self.sets = sets or {}


class Instrumenter:
    def __init__(self, name, stages, driver_args):
        self.name = name
        self.stages = stages
        self.driver_args = driver_args


def run_process(command, timeout=None, capture_output=False):
    if timeout is not None and timeout <= 0:
        print(f"Time limit reached before command: {' '.join(command)}")
        return None, "", ""

    try:
        process = subprocess.Popen(
            command,
            start_new_session=True,
            stdout=subprocess.PIPE if capture_output else None,
            stderr=subprocess.PIPE if capture_output else None,
            text=capture_output,
        )
    except Exception as e:
        print(f"Command failed to start: {' '.join(command)} - {str(e)}")
        return None, "", str(e)

    try:
        stdout, stderr = process.communicate(timeout=timeout)
        return process.returncode, stdout, stderr
    except subprocess.TimeoutExpired:
        try:
            os.killpg(os.getpgid(process.pid), signal.SIGKILL)
        except (ProcessLookupError, PermissionError):
            try:
                process.kill()
            except Exception:
                pass
        stdout, stderr = process.communicate()
        print(f"Command timed out: {' '.join(command)}")
        return None, stdout, stderr


def execute_command(command, timeout=None):
    return_code, _, _ = run_process(command, timeout)
    return return_code == 0


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
                new_content.append(f"goto goal_{label_count};\n")
                new_content.append(f"goal_{label_count}:;\n")
                new_content.append(f"__testcoca_process_goal({label_count});\n")
                label_count += 1
            else:
                new_content.append(line)

        with open(input_file, 'w') as f:
            f.writelines(new_content)

        return True
    except Exception as e:
        print(f"Error processing {input_file}: {str(e)}")
        return False


def stage_transform_goals(ctx):
    return transform_goals(ctx["rewritten_c"])


# Reusable stages. "src_c" is the C source currently flowing through the
# pipeline: it starts as the user's input and is replaced whenever a stage
# rewrites the source.
CLANG_EMIT_LL = Stage(tool="clang", args=[
    "-O0", "-S", "-emit-llvm",
    "-Wno-everything", "-fbracket-depth=1024",
    "{cc_arch}", "{src_c}", "-o", "{ll}",
])
AST_REWRITE = Stage(
    tool="ast_instrumenter",
    args=["{src_c}", "{rewritten_c}"],
    sets={"src_c": "rewritten_c"},
)
LABEL_ADDER = Stage(
    tool="label_adder",
    args=["--labels-branching-only", "--labels-switch-only",
          "--labels-ternary-only", "{src_c}"],
    stdout_to="{rewritten_c}",
    sets={"src_c": "rewritten_c"},
)
TRANSFORM_GOALS = Stage(func=stage_transform_goals)
IR_BRANCH = Stage(tool="ir_instrumenter", args=[
    "--inst_br", "--input", "{ll}", "--output", "{instrumented_ll}",
])
IR_GOALS = Stage(tool="ir_instrumenter", args=[
    "--input", "{ll}", "--output", "{instrumented_ll}", "--inst_goals",
])
IR_RENAME_ONLY = Stage(tool="ir_instrumenter", args=[
    "--input", "{ll}", "--output", "{instrumented_ll}",
])
IR_ERROR = Stage(tool="ir_instrumenter", args=[
    "--inst_err", "{error_func}", "--input", "{ll}", "--output", "{instrumented_ll}",
])

INSTRUMENTERS = {
    "branch": Instrumenter(
        "branch",
        [CLANG_EMIT_LL, IR_BRANCH],
        ["--goal", "coverage"],
    ),
    "testcomp": Instrumenter(
        "testcomp",
        [LABEL_ADDER, TRANSFORM_GOALS, CLANG_EMIT_LL, IR_GOALS],
        ["--testcomp", "--goal", "coverage"],
    ),
    "decision": Instrumenter(
        "decision",
        [AST_REWRITE, CLANG_EMIT_LL, IR_RENAME_ONLY],
        ["--testcomp", "--goal", "coverage"],
    ),
    "error": Instrumenter(
        "error",
        [CLANG_EMIT_LL, IR_ERROR],
        ["--goal", "call"],
    ),
}


def resolve_tools(self_dir):
    return {
        "clang": "clang",
        "clang++": "clang++",
        "ir_instrumenter": os.path.join(self_dir, "tools", "@IR_INSTRUMENTER_FILE@"),
        "ast_instrumenter": os.path.join(self_dir, "tools", "@AST_INSTRUMENTER_FILE@"),
        "driver": os.path.join(self_dir, "tools", "@DRIVER_FILE@"),
        "label_adder": os.path.join(self_dir, "label-adder"),
    }


def build_context(config):
    input_c = config.input_file
    base = os.path.splitext(os.path.basename(input_c))[0]
    return {
        "src_c": input_c,
        "rewritten_c": os.path.join(config.output_dir, base + "_rewritten.c"),
        "ll": config.ll_file,
        "instrumented_ll": config.instrumented_ll,
        "target": config.target_file,
        "output_dir": config.output_dir,
        "base": base,
        "cc_arch": ["-m32"] if config.use_m32 else [],
        "error_func": config.error_func,
        "timeout": config.command_timeout(),
    }


def render_args(tokens, ctx):
    rendered = []
    for token in tokens:
        if isinstance(token, list):
            rendered.extend(str(item) for item in token)
            continue
        if not isinstance(token, str):
            rendered.append(str(token))
            continue

        whole = re.fullmatch(r"\{(\w+)\}", token)
        if whole and isinstance(ctx.get(whole.group(1)), list):
            rendered.extend(str(item) for item in ctx[whole.group(1)])
            continue

        rendered.append(
            re.sub(r"\{(\w+)\}", lambda m: str(ctx[m.group(1)]), token))
    return rendered


def run_stage(stage, ctx, tools):
    if stage.func is not None:
        return stage.func(ctx)

    cmd = [tools[stage.tool]] + render_args(stage.args, ctx)
    timeout = ctx.get("timeout")

    if stage.stdout_to is not None:
        output_path = render_args([stage.stdout_to], ctx)[0]
        return_code, stdout, _ = run_process(cmd, timeout, capture_output=True)
        if return_code != 0 and not stdout:
            print(f"Command failed: {' '.join(cmd)}")
            return False
        with open(output_path, 'w') as f:
            f.write(stdout or "")
        return True

    return execute_command(cmd, timeout)


def run_pipeline(stages, ctx, tools):
    for stage in stages:
        if not run_stage(stage, ctx, tools):
            return False
        for dest, source in stage.sets.items():
            ctx[dest] = ctx[source]
    return True


def build(config):
    if config.format:
        input_filename = os.path.basename(config.input_file)
        input_name, _ = os.path.splitext(input_filename)
        formatted_path = os.path.join(config.output_dir, f"{input_name}_tidy.c")

        with open(config.input_file, 'r') as src, open(formatted_path, 'w') as dst:
            dst.write(src.read())

        clang_tidy_cmd = ["clang-tidy",
                          "--checks=-*,readability-braces-around-statements",
                          "--fix-errors",
                          formatted_path
                          ]

        if not config.silent:
            print("Running clang-tidy...")

        run_process(clang_tidy_cmd, config.command_timeout(), capture_output=True)
        config.input_file = formatted_path

    instrumenter = INSTRUMENTERS[config.mode]
    ctx = build_context(config)

    if not config.silent:
        print(f"Instrumenting ({instrumenter.name})...")
    if not run_pipeline(instrumenter.stages, ctx, config.tools):
        print(f"Instrumentation failed: {config.input_file}")
        return False

    target_libs = [
                      os.path.join(config.self_dir, "lib32" if config.use_m32 else "lib", lib)
                      for lib in @TARGET_LIBRARIES_FILES_LIST@
    ]

    cmd = ["clang++"] + (["-m32"] if config.use_m32 else []) + [
        "-O3", config.instrumented_ll
    ] + "@TARGET_NEEDED_COMPILATION_FLAGS@".split() + target_libs + [
              "-o", config.target_file
          ]

    if not config.silent: print("Linking executable...")
    if not execute_command(cmd, config.command_timeout()):
        print(f"Linking failed: {config.input_file}")
        return False

    if not config.silent: print("Build completed successfully")
    return True


def run_tests(config, remaining_ms):
    if not os.path.exists(config.target_file):
        old_target = os.path.join(os.path.dirname(config.input_file), os.path.basename(config.target_file))
        if not os.path.exists(old_target):
            print(f"Target executable not found: {config.target_file}")
            return False
        target = old_target
    else:
        target = config.target_file

    cmd = ([
               config.tools["driver"],
               "--path_to_target", target,
               "--test_dir", config.test_suite,
               "--output_dir", config.output_dir,
               "--max_exec_milliseconds", str(remaining_ms),
           ]
           + INSTRUMENTERS[config.mode].driver_args
           + config.options)

    if not config.silent: print("Running tests...")
    return execute_command(cmd, remaining_ms / 1000.0 + DRIVER_GRACE_SECONDS)


def parse_coverage_goal(goal_file):
    if not goal_file:
        print("Using default branch coverage")
        return "branch", ""

    try:
        with open(goal_file, 'r') as f:
            content = f.read().strip()
    except Exception as e:
        print(f"Error reading goal file: {str(e)}")
        return None, ""

    # taken from https://gitlab.com/sosy-lab/software/test-suite-validator/-/blob/main/suite_validation/__init__.py
    pattern = r"COVER\s*\(\s*init\s*\(\s*main\s*\(\s*\)\s*\)\s*,\s*FQL\s*\(COVER\s+EDGES\s*\((.*)\)\s*\)\s*\)"
    match = re.search(pattern, content)
    if not match:
        print("Invalid coverage goal specification")
        return None, ""

    goal_type = match.group(1).strip()
    if goal_type.startswith("@CALL"):
        err_func = goal_type[6:-1]
        return "error", err_func
    elif goal_type == "@DECISIONEDGE":
        return "branch", ""

    print(f"Unsupported coverage type: {goal_type}")
    return None, ""


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


def write_fallback_result(output_dir, stop_reason="time_budget_depleted"):
    os.makedirs(output_dir, exist_ok=True)
    result_path = os.path.join(output_dir, "result.json")
    with open(result_path, "w") as f:
        json.dump({
            "coverage": 0.0,
            "coverage_map": {},
            "time_limit_reached": True,
            "elapsed_ms": 0,
            "tests_executed": 0,
            "tests_skipped": 0,
            "stop_reason": stop_reason,
        }, f)
    return result_path


def print_help(self_dir):
    print("TestCoCa Usage")
    print("==============")
    print("--help                 Show this help")
    print("--input_file <PATH>    Source C file to build and test")
    print("--test_suite <PATH>    Directory containing tests")
    print("--output_dir <PATH>    Directory for results (default: current dir)")
    print("--goal <PATH>          .prp file for coverage goal (default: branch)")
    print("--max_exec_milliseconds <MS>  Wall-clock time limit for the whole invocation")
    print("--testcomp             Use TestCov's label-adder to calculate branch coverage")
    print("--decision-coverage    Instrument decisions at the AST level (break/switch aware)")
    print("--format               Use clang-tidy to format the source file")
    print("--skip_building        Skip building step")
    print("--skip_testing         Skip testing step")
    print("--silent_mode          Suppress output messages")
    print("--m32                  Compile for 32-bit architecture")


def select_mode(config, goal_mode):
    if goal_mode == "error":
        if config.testcomp or config.decision_coverage:
            print("WARNING: Ignoring coverage-mode flags, not applicable to cover-error category")
        return "error"

    if config.testcomp and config.decision_coverage:
        print("--testcomp and --decision-coverage are mutually exclusive")
        return None
    if config.testcomp:
        return "testcomp"
    if config.decision_coverage:
        return "decision"
    return goal_mode


def main():
    start_time = time.monotonic()
    self_dir = os.path.dirname(os.path.abspath(__file__))
    config = Config(self_dir)
    config.tools = resolve_tools(self_dir)
    goal_file = None

    i = 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg == "--help":
            print_help(self_dir)
            return 0
        elif arg == "--version":
            print("1.0")
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
        elif arg == "--max_exec_milliseconds" and i + 1 < len(sys.argv):
            try:
                config.max_exec_milliseconds = max(0, int(sys.argv[i + 1]))
            except ValueError:
                print(f"Invalid --max_exec_milliseconds value: {sys.argv[i + 1]}")
                return 1
            i += 1
        elif arg == "--skip_building":
            config.skip_building = True
        elif arg == "--skip_testing":
            config.skip_testing = True
        elif arg == "--testcomp":
            config.testcomp = True
        elif arg == "--decision-coverage":
            config.decision_coverage = True
        elif arg == "--format":
            config.format = True
        elif arg == "--m32":
            config.use_m32 = True
        else:
            config.options.append(arg)
        i += 1

    config.deadline = start_time + config.max_exec_milliseconds / 1000.0

    if not config.input_file or not os.path.isfile(config.input_file):
        print("Missing or invalid input file")
        return 1

    goal_mode, config.error_func = parse_coverage_goal(goal_file)
    if goal_mode is None:
        print("Invalid coverage goal configuration")
        return 1

    config.mode = select_mode(config, goal_mode)
    if config.mode is None:
        return 1

    base = os.path.splitext(os.path.basename(config.input_file))[0]
    config.ll_file = os.path.join(config.output_dir, base + ".ll")
    config.instrumented_ll = os.path.join(config.output_dir, base + "_instrumented.ll")
    config.target_file = os.path.join(config.output_dir, base + "_target")

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

        if config.remaining_seconds(FINALISATION_RESERVE_SECONDS) <= 0:
            print("Time limit reached before test-suite extraction")
            write_fallback_result(config.output_dir)
            return 0

        prepared_test_suite = prepare_test_suite(config.test_suite, config.output_dir)
        if not prepared_test_suite:
            return 1
        config.test_suite = prepared_test_suite

        remaining_ms = int(max(
            0, config.remaining_seconds(FINALISATION_RESERVE_SECONDS) * 1000))

        if remaining_ms <= 0:
            print("Time limit reached before testing")
            write_fallback_result(config.output_dir)
            return 0

        if not config.silent: print(f"Using test suite: {config.test_suite}")
        if not run_tests(config, remaining_ms):
            result_path = os.path.join(config.output_dir, "result.json")
            if not os.path.exists(result_path):
                write_fallback_result(config.output_dir, "driver_failed")
            print("Testing failed")
            return 1

    if not config.silent:
        print(f"Completed successfully in {time.monotonic() - start_time:.2f} seconds")
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
