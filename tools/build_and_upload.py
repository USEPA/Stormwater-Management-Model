# -*- coding: utf-8 -*-
"""Script used in appveyor CI for building and uploading packages."""

# Standard library imports
import os
import subprocess


def main():
    """Build and upload a package if on develop and PR is merged."""
    pr_number = os.environ.get('APPVEYOR_PULL_REQUEST_NUMBER')
    branch= os.environ.get('APPVEYOR_REPO_BRANCH')
    token = os.environ.get('SWWM_CI_UPLOAD_TOKEN')
    python = os.environ.get('PYTHON_VERSION')

    print([pr_number, branch])

    if branch == 'develop' and pr_number is None:
        cmd = ['conda-build', 'conda.recipe', '--user', 'owa',
               '--token', token, '--old-build-string', '--python', python]
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()
        stdout = stdout.decode()
        stderr = stderr.decode()
        print(stdout)
        print(stderr)


if __name__ == '__main__':
    main()
