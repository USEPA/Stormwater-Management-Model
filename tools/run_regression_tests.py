# -*- coding: utf-8 -*-
"""Run regression tests."""

# Standard library imports
import io
import json
import subprocess
import os
import shutil
import sys


# Reference
SWMM_REF_VERSION = '5112'
SWMM_EXEC = 'run-swmm'

# Ensure proper cross os / \\ handling and allow running independent of pwd
HERE = os.path.abspath(os.path.dirname(__file__))
REPO_ROOT = os.path.dirname(HERE)
PACKAGES_PATH = os.path.join(REPO_ROOT, 'build', 'packages', 'nrtest',
                             'scripts')
TESTS_PATH = os.path.join(REPO_ROOT, 'tests')
NRTESTSUITE_PATH = os.path.join(TESTS_PATH, 'swmm-nrtestsuite')
APPS_PATH = os.path.join(NRTESTSUITE_PATH, 'apps')
TEST_EXAMPLES_PATH = os.path.join(NRTESTSUITE_PATH, 'tests', 'examples')
BENCHARK_PATH = os.path.join(NRTESTSUITE_PATH, 'benchmark')
REF_BENCHMARK_PATH = os.path.join(BENCHARK_PATH, 'swmm-' + SWMM_REF_VERSION)

# OS identifiers
MAC = sys.platform == 'darwin'
WIN = os.name == 'nt'
LINUX = sys.platform.startswith('linux')
PY3 = sys.version_info[0] == 3


def run_command(cmd, print_command=True):
    """Run command in pipes and return stdout, sterr and return code."""
    stdout, stderr, code = None, None, None
    cmd = [str(c) for c in cmd]
    if print_command:
        print('\nINFO: ' + ' '.join(cmd) + '\n')

    try:
        p = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        stdout, stderr = p.communicate()
        code = p.returncode
        if PY3:
            stdout = stdout.decode()
            stderr = stderr.decode()
    except Exception as e:
        print(e)

    return stdout, stderr, code


def generate_config_data(executable_location):
    """Generate configuration file for running regression tests."""
    path = os.path.abspath(executable_location)

    if LINUX or MAC:
        test_cmd = os.path.join(path, SWMM_EXEC)
    elif WIN:
        # nrtests works with unix style separators on windows
        test_cmd = os.path.join(path, SWMM_EXEC + '.exe').replace('\\', '/')
    else:
        raise Exception('Machine not supported!')

    # Check executable existence
    if not os.path.isfile(test_cmd):
        raise Exception('SWMM executable not found on location')

    version = ""
    build_description = ""
    data = {
        "name": "swmm",
        "version": version,
        "description": build_description,
        "setup_script": "",
        "exe": test_cmd,
    }
    return data


def run_tests(executable_location, testid, rtol=0.01, atol=0.0):
    """Run regression tests."""
    test_app_path = os.path.join(APPS_PATH, 'swmm-' + testid + '.json')
    test_output_path = os.path.join(BENCHARK_PATH, "swmm-" + testid)

    # Clean any previous runs
    if os.path.isdir(test_output_path):
        print("\n\n\nINFO: Removing previous run data\n")
        shutil.rmtree(test_output_path)

    # Generate configuration data and write to disk
    print('\n\n\nINFO: Generating configuration data:\n')
    data = generate_config_data(executable_location)
    print(data)
    with io.open(test_app_path, 'wb') as f_handle:
        json.dump(data, f_handle, indent=4, sort_keys=True)

    pyexe = 'python'
    nrtest_script = 'nrtest'
    python_exe = []
    if WIN:
        pyexe = 'python.exe'
        nrtest_script = os.path.join(PACKAGES_PATH, 'nrtest')
        python_exe = [os.path.join(sys.prefix, pyexe)]

    # Generate commands
    exec_cmd = python_exe + [
        nrtest_script,
        'execute',
        test_app_path,
        TEST_EXAMPLES_PATH,
        '-o',
        test_output_path,
    ]
    compare_cmd = python_exe + [
        nrtest_script,
        'compare',
        test_output_path,
        REF_BENCHMARK_PATH,
        '--rtol',
        rtol,
        '--atol',
        atol,
    ]
    print('\n\n\nINFO: Running nrtest execute command:\n')
    out, err, code = run_command(exec_cmd)
    print(err)
    print(out)

    if code == 0:
        print('\n\n\nINFO: Running nrtest compare command:\n')
        out, err, code = run_command(compare_cmd)
        print(err)
        print(out)

        if code != 0:
            print('\n\n\nERROR: Comparison against benchmark failed!\n')
            sys.exit(code)
    else:
        print('\n\n\nERROR: Generation of benchmark failed!\n')
        sys.exit(code)


if __name__ == '__main__':
    args = sys.argv[1:]
    if args and len(args) == 2:
        executable_location, test_id = args
        run_tests(executable_location, test_id)
    else:
        print('ERROR: Need to provide `path to swmm exec folder` and `testid`')
