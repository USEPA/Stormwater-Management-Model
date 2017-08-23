# -*- coding: utf-8 -*-
"""Run clang formatter on a file or directory."""

# yapf: disable

# Standard library imports
import argparse
import difflib
import os
import subprocess
import sys


# yapf: enable

PY3 = sys.version_info[0] == 3
WIN = sys.platform.startswith('win32')
CLANG_FORMAT_EXE = 'clang-format.exe' if WIN else 'clang-format'
STYLES = ['chromium', 'file', 'google', 'llvm', 'mozilla', 'webkit']


# Update this list as new PRs adapt the code on new files
FILES_FAILING_FORMAT_CHECK = [
    'src/climate.c',
    'src/consts.h',
    'src/controls.c',
    'src/culvert.c',
    'src/datetime.c',
    'src/datetime.h',
    'src/dwflow.c',
    'src/dynwave.c',
    'src/enums.h',
    'src/error.c',
    'src/error.h',
    'src/exfil.c',
    'src/exfil.h',
    'src/findroot.c',
    'src/findroot.h',
    'src/flowrout.c',
    'src/forcmain.c',
    'src/funcs.h',
    'src/gage.c',
    'src/globals.h',
    'src/gwater.c',
    'src/hash.c',
    'src/hash.h',
    'src/headers.h',
    'src/hotstart.c',
    'src/iface.c',
    'src/infil.c',
    'src/infil.h',
    'src/inflow.c',
    'src/input.c',
    'src/inputrpt.c',
    'src/keywords.c',
    'src/keywords.h',
    'src/kinwave.c',
    'src/landuse.c',
    'src/lid.c',
    'src/lid.h',
    'src/lidproc.c',
    'src/link.c',
    'src/macros.h',
    'src/massbal.c',
    'src/mathexpr.c',
    'src/mathexpr.h',
    'src/mempool.c',
    'src/mempool.h',
    'src/node.c',
    'src/objects.h',
    'src/odesolve.c',
    'src/odesolve.h',
    'src/output.c',
    'src/project.c',
    'src/qualrout.c',
    'src/rain.c',
    'src/rdii.c',
    'src/report.c',
    'src/roadway.c',
    'src/routing.c',
    'src/runoff.c',
    'src/snow.c',
    'src/stats.c',
    'src/statsrpt.c',
    'src/subcatch.c',
    'src/surfqual.c',
    'src/swmm5.c',
    'src/swmm5.h',
    'src/table.c',
    'src/text.h',
    'src/toolkitAPI.c',
    'src/toolkitAPI.h',
    'src/toposort.c',
    'src/transect.c',
    'src/treatmnt.c',
    'src/xsect.c'
]


def format_file(filepath, style='file', inplace=False):
    """"""
    if not os.path.isfile(filepath):
        print('ERROR: File "{}" not found!\n'.format(filepath))

    if style not in STYLES and style != 'file':
        raise Exception('Unknow style used.')

    # Avoid flashing an ugly cmd prompt on Windows when invoking clang-format
    startupinfo = None
    if WIN:
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startupinfo.wShowWindow = subprocess.SW_HIDE

    # Call formatter
    args = [CLANG_FORMAT_EXE, filepath, '-style', style]

    try:
        p = subprocess.Popen(
            args,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            stdin=subprocess.PIPE,
            startupinfo=startupinfo,
        )

        stdout, stderr = p.communicate()
    except Exception:
        print('ERROR: clang-format executable not found!')
        return -1

    if stderr:
        print('ERROR: Formatting failed!')
        print(stderr)

    if not stdout:
        print(
            'ERROR: No output from clang-format (crashed?).\n'
            'Please report to bugs.llvm.org.'
        )
    else:
        # Make the diff of what changed
        with open(filepath, 'r') as f:
            data = f.read()

        if inplace:
            try:
                p2 = subprocess.Popen(
                    args + ['-i'],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    stdin=subprocess.PIPE,
                    startupinfo=startupinfo,
                )
                _stdout, _stderr = p2.communicate()
            except Exception:
                pass

        lines = data.split('\n')
        if PY3:
            stdout = stdout.decode()
        new_lines = stdout.split('\n')
        diff = difflib.unified_diff(
            lines, new_lines, fromfile=filepath, tofile=filepath
        )
        diff_lines = [line for line in diff]

    return diff_lines


def files(root_path, extensions=('cpp', 'c', 'h')):
    """Search for all files in `root_path` matching `extensions`."""
    paths = []
    for dir_name, subdir_list, file_list in os.walk(root_path):
        for fname in file_list:
            path = os.path.join(dir_name, fname)
            ext = path.rsplit('.')[-1]
            if extensions and ext in extensions:
                paths.append(path)
    return paths


def parse_arguments():
    """Parse CLI arguments."""
    parser = argparse.ArgumentParser(
        description='A tool to format C/C++/Java/JavaScript/'
        'Objective-C/Protobuf code.'
    )

    parser.add_argument(
        'folder',
        help="Folder with files to format. Files are searched recursively"
    )
    parser.add_argument(
        '-s',
        '--style',
        action="store",
        dest="style",
        default='file',
        choices=STYLES,
        help="Style to use. By default uses `file` and expetcs .clang-format"
    )
    parser.add_argument(
        '-i',
        '--inplace',
        action="store_const",
        dest="inplace",
        const=True,
        help="Replace inplace the formating on selected files in path"
    )
    options = parser.parse_args()

    return options


def run_process():
    """Run main process."""
    options = parse_arguments()
    fullpath = options.folder
    if os.path.isfile(fullpath):
        paths = [fullpath]
    else:
        paths = files(fullpath)
    file_errors = []

    for path in paths:
        diff_lines = format_file(path, inplace=options.inplace)
        if diff_lines == -1:
            return

        std_path = path.replace('\\', '/')
        if diff_lines and std_path not in FILES_FAILING_FORMAT_CHECK:
            file_errors.append(path)
            print('*' * len(path))
            print(path)
            print('*' * len(path))
            # Avoid the long prints on CI while the style is settled
            print('\n'.join(diff_lines))
            print('\n\n')

    # If any errors found exit with code
    if file_errors:
        print('ERROR: The following files failed the clang-format test:\n')
        for filepath in file_errors:
            print('\t' + filepath)
        sys.exit(1)
    else:
        print('SUCCESS: All files passed the clang-format test!')
    print('\n')


if __name__ == "__main__":
    run_process()
