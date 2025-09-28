#!/usr/bin/env python3
import os
import sys
import subprocess
import shutil
import json


def _execute(command_and_args, timeout_ = None):
    cmd = [x for x in command_and_args if len(x) > 0]
    return subprocess.run(cmd, timeout=timeout_)


def help(self_dir):
    print("Building TestCoCa")
    print("=================")
    print("The script used for fast building of TestCoCa.")
    print("Quick use:")
    print("  python3 ./build.py")
    print("Usage:")
    print("  python3 ./build.py [OPTIONS]")
    print("Options:")
    print("  help                 Prints this help message.")
    print("  config <TYPE>        The type of configuration to built. <TYPE> can be one of these:")
    print("                       Debug, Release or RelWithDebInfo. Default value is Release.")
    print("  clean                When specified, folders 'build' and 'dist' will be removed before build.")
    print("                       Otherwise, all fizzer's modules will be built for 64-bit machine.")
    print("  m32                  When specified, only libraries necessary for compiling")
    print("                       the source C files will be compiled for 32-bit machine (cpu).")
    print("                       Otherwise, all fizzer's modules will be built, i.e., for both")
    print("                       32-bit and 64-bit machine.")
    print("NOTE: It is assumed that 'Clang' and 'Ninja' are installed in the system.")
    print("      It is further assumed 'boost' and 'llvm' libraries are installed in 'vcpkg'")
    print("      package manager or directly in the system (not recommended option).")
    print("More information can be found in the ./README.md file.")

def init(build_dir, build_config):
    os.chdir(build_dir)
    _execute(
        [ "cmake",
            "..",
            "--no-warn-unused-cli",
            "-G Ninja",
            "-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE",
            "-DCMAKE_C_COMPILER:FILEPATH=clang",
            "-DCMAKE_CXX_COMPILER:FILEPATH=clang++",
            "-DCMAKE_BUILD_TYPE:STRING=" + build_config ]
        )


def build(build_dir, build_config, use_m32):
    os.chdir(build_dir)
    _execute(
        [ "cmake",
            "..",
            "--no-warn-unused-cli",
            "-G Ninja",
            "-DBUILD_LIBS_32_BIT=" + ("No" if use_m32 is False else "Yes"),
            "-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE",
            "-DCMAKE_C_COMPILER:FILEPATH=clang",
            "-DCMAKE_CXX_COMPILER:FILEPATH=clang++",
            "-DCMAKE_BUILD_TYPE:STRING=" + build_config ]
        )
    _execute(["cmake", "--build", ".", "--config", build_config, "--target", "install"])


def main():
    self_dir = os.path.normpath(os.path.abspath(os.path.dirname(__file__)))
    build_dir = os.path.join(self_dir, "build")
    install_dir = os.path.join(self_dir, "dist")
    build_config = "Release" # "Debug" "Release" "RelWithDebInfo"
    do_clean = False
    only_m32 = False

    i = 1
    while (i < len(sys.argv)):
        arg = sys.argv[i]
        if arg == "--help":
            help(self_dir)
            return
        elif arg == "--config" and i+1 < len(sys.argv) and sys.argv[i+1] in ["Debug", "Release", "RelWithDebInfo"]:
            build_config = sys.argv[i+1]
            i += 1
        elif arg == "--clean":
            do_clean = True
        elif arg == "--m32":
            only_m32 = True
        i += 1

    if do_clean is True:
        if (os.path.isdir(build_dir)):
            shutil.rmtree(build_dir)
        if (os.path.isdir(install_dir)):
            shutil.rmtree(install_dir)

    os.makedirs(install_dir, exist_ok=True)
    if not os.path.isdir(build_dir):
        os.makedirs(build_dir, exist_ok=True)
        init(build_dir, build_config)

    build(build_dir, build_config, True)
    if only_m32 is False:
        build(build_dir, build_config, False)


if __name__ == "__main__":
    exit_code = 0
    old_cwd = os.path.abspath(os.getcwd())
    try:
        main()
    except Exception as e:
        exit_code = 1
        print("ERROR: " + str(e))
    finally:
        os.chdir(old_cwd)
    exit(exit_code)
