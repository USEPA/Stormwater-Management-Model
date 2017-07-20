# -*- coding: utf-8 -*-
"""Run clang formatter on a file or directory."""

# Standard library imports
import argparse
import difflib
import os
import subprocess
import sys


PY3 = sys.version_info[0] == 3
WIN = sys.platform.startswith('win32')
CLANG_FORMAT_EXE = 'clang-format.exe' if WIN else 'clang-format'
STYLES = ['chromium', 'file', 'google', 'llvm', 'mozilla', 'webkit']


def format_file(filepath, style='file'):
    """"""
    if not os.path.isfile(filepath):
        print('ERROR: File "{}" not found!\n'.format(filepath))
        
    if style not in STYLES and style != 'file':
        raise Exception('Unknow style used.')

    # Avoid flashing an ugly cmd prompt on Windows when invoking clang-format.

    startupinfo = None
    if WIN:
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startupinfo.wShowWindow = subprocess.SW_HIDE

    # Call formatter.
    args = [CLANG_FORMAT_EXE, filepath, '-style', style]

    p = subprocess.Popen(
        args,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        stdin=subprocess.PIPE,
        startupinfo=startupinfo,
    )

    stdout, stderr = p.communicate()

    if stderr:
      print('Formatting failed!')

    if not stdout:
        print('No output from clang-format (crashed?).\n'
              'Please report to bugs.llvm.org.')
    else:
        # Make the diff of what changed
        with open(filepath, 'r') as f:
            data = f.read()
        
        lines = data.split('\n')
        if PY3:
            stdout = stdout.decode()
        new_lines = stdout.split('\n')
        diff = difflib.unified_diff(lines, new_lines, fromfile=filepath,
                                    tofile=filepath)
        diff_lines = [line for line in diff]

    return diff_lines


def files(root_path, extensions=('cpp', 'c', 'h')):
    """Search recursively for all files in `root_path` matching `extensions`."""
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
        'Objective-C/Protobuf code.')

    parser.add_argument(
        'folder',
        help="Folder with files to format. Files are searched recursively.")
    parser.add_argument(
        '-s',
        '--style',
        action="store",
        dest="style",
        default='file',
        choices=STYLES,
        help="Style to use. By default uses `file` and expetcs .clang-format")
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
        diff_lines = format_file(path)
        if diff_lines:
            file_errors.append(path)
            print('*'*len(path))
            print(path)
            print('*'*len(path))
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
