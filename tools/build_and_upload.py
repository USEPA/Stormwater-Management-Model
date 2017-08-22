# -*- coding: utf-8 -*-
"""Script used in appveyor CI for building and uploading packages."""

# Standard library imports
import os
import subprocess


def main():
    pr_number = os.environ.get('APPVEYOR_PULL_REQUEST_NUMBER')
    branch= os.environ.get('APPVEYOR_REPO_BRANCH')
    cmd_in_env = os.environ.get('CMD_IN_ENV')
    token = os.environ.get('SWWM_CI_UPLOAD_TOKEN')

    print([pr_number, branch, cmd_in_env])

    if branch == 'develop' and pr_number is None:
        cmd = [cmd_in_env, 'conda-build', 'conda.recipe', '--user', 'owa',
               '--token', token]
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()
        stdout = stdout.decode()
        stderr = stderr.decode()
        print(stdout)
        print(stderr)


if __name__ == '__main__':
    main()
