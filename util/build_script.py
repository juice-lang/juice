#!/usr/bin/env python3
#
# util/build_script.py - Python script for building the juice compiler
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2020 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


import argparse
import os
import re
import shutil
import subprocess
import sys

from enum import Enum


class GitRemoteStatus(Enum):
    error = 0
    equal = 1
    remote_ahead = 2
    local_ahead = 3
    diverged = 4


class LLVMStatus(Enum):
    error = 0
    up_to_date = 1
    needs_build = 2


def _find_getch():
    try:
        import termios
    except ImportError:
        # Non-POSIX. Return msvcrt's (Windows') getch.
        import msvcrt
        return msvcrt.getch

    # POSIX system. Create and return a getch that manipulates the tty.
    import sys
    import tty

    def _getch():
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch

    return _getch


getch = _find_getch()


def job_count() -> int:
    count = os.cpu_count()

    if count is not None:
        return count + 1

    return 3


git_repos = []


def is_git_repo(cwd: str) -> bool:
    if cwd in git_repos:
        return True
    if subprocess.call(['git', 'rev-parse'], cwd=cwd) == 0:
        git_repos.append(cwd)
        return True
    return False


def git_clone(url: str, cwd: str) -> bool:
    if subprocess.call(['git', 'clone', url, cwd]) == 0:
        return True
    return False


def git_update(cwd: str) -> bool:
    if is_git_repo(cwd):
        if subprocess.call(['git', 'remote', 'update'], cwd=cwd) == 0:
            return True
    return False


def git_remote_status(cwd: str) -> GitRemoteStatus:
    if is_git_repo(cwd):
        local_process = subprocess.run(['git', 'rev-parse', '@'],
                                       stdout=subprocess.PIPE, cwd=cwd)
        if local_process.returncode != 0:
            return GitRemoteStatus.error
        local_hash = local_process.stdout

        remote_process = subprocess.run(['git', 'rev-parse', '@{u}'],
                                        stdout=subprocess.PIPE, cwd=cwd)
        if remote_process.returncode != 0:
            return GitRemoteStatus.error
        remote_hash = remote_process.stdout

        base_process = subprocess.run(['git', 'merge-base', '@', '@{u}'],
                                      stdout=subprocess.PIPE, cwd=cwd)
        if base_process.returncode != 0:
            return GitRemoteStatus.error
        base_hash = base_process.stdout

        if local_hash == remote_hash:
            return GitRemoteStatus.equal
        elif local_hash == base_hash:
            return GitRemoteStatus.remote_ahead
        elif remote_hash == base_hash:
            return GitRemoteStatus.local_ahead
        else:
            return GitRemoteStatus.diverged
    return GitRemoteStatus.error


def git_pull(cwd: str) -> bool:
    if is_git_repo(cwd):
        return subprocess.call(['git', 'pull'], cwd=cwd) == 0
    return False


def git_has_uncommitted_changes(cwd: str) -> bool:
    if is_git_repo(cwd):
        if subprocess.check_output(['git', 'status', '--porcelain'], cwd=cwd):
            return True
    return False


def git_get_current_hash(cwd: str) -> bytes:
    if is_git_repo(cwd):
        return subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=cwd)
    return b''


def cmake(source_dir: str, install_dir: str, cwd: str) -> bool:
    cmake_process = subprocess.run(['cmake', source_dir, '-G', 'Unix Makefiles',
                                    '-D', 'CMAKE_INSTALL_PREFIX=' + install_dir,
                                    '-D', 'CMAKE_BUILD_TYPE=' + args.build_type],
                                   stdout=subprocess.PIPE, cwd=cwd)
    if cmake_process.returncode != 0:
        return False
    cmake_output = cmake_process.stdout
    return True


def make_and_install(cwd: str) -> bool:
    make_process = subprocess.Popen(['make', '-j', str(job_count())],
                                    stdout=subprocess.PIPE, cwd=cwd,
                                    universal_newlines=True)
    while True:
        output = make_process.stdout.readline().strip()
        match = re.match(r'^\[\s*(\d+)%\]', output)
        if match is not None:
            progress = match.group(1)
            print('\rProgress: ' + progress, end='%')
        returncode = make_process.poll()

        if returncode is not None:
            print('')
            print('Done!\n')
            if returncode != 0:
                return False
            break

    print('Installing')

    make_install_process = subprocess.Popen(['make', 'install'],
                                            stdout=subprocess.PIPE, cwd=cwd,
                                            universal_newlines=True)
    while True:
        output = make_install_process.stdout.readline().strip()
        if output:
            print(output)
        returncode = make_install_process.poll()

        if returncode is not None:
            if returncode != 0:
                return False
            break


parser = argparse.ArgumentParser(prog='juice_build_script',
                                 description='Build the juice-lang compiler',
                                 epilog='Defaults to building in release mode.')

build_type_group = parser.add_mutually_exclusive_group()
build_type_group.add_argument('-d',
                              '--debug',
                              action='store_const',
                              const='Debug',
                              dest='build_type',
                              help='build in debug mode')
build_type_group.add_argument('-r',
                              '--release',
                              action='store_const',
                              const='Release',
                              dest='build_type',
                              help='build in release mode')

parser.add_argument('--no-update',
                    action='store_false',
                    dest='should_update',
                    help="don't auto-update llvm before building")
parser.add_argument('--no-rebuild',
                    action='store_false',
                    dest='should_rebuild',
                    help="don't rebuild the juice project completely")
parser.add_argument('--rebuild-llvm',
                    action='store_true',
                    dest='should_rebuild_llvm',
                    help='force rebuilding of the llvm libraries')

parser.set_defaults(build_type='Release')

args = parser.parse_args()


util_dir = os.path.dirname(os.path.realpath(__file__))
repo_dir = os.path.dirname(util_dir)
root_dir = os.path.dirname(repo_dir)

build_dir = os.path.join(repo_dir, 'build')
cmake_dir = os.path.join(build_dir, 'cmake')
bin_dir = os.path.join(build_dir, 'bin')

llvm_repo_dir = os.path.join(root_dir, 'juice-llvm')
llvm_build_dir = os.path.join(llvm_repo_dir, 'build')
llvm_install_dir = os.path.join(llvm_build_dir, 'install')
llvm_last_build_hash_file = os.path.join(llvm_build_dir, '.lastbuildhash')


def llvm_status() -> LLVMStatus:
    if is_git_repo(llvm_repo_dir):
        if not git_has_uncommitted_changes(llvm_repo_dir):
            if os.path.isfile(llvm_last_build_hash_file):
                with open(llvm_last_build_hash_file, 'rb') as f:
                    last_build_hash = f.read()
                    current_hash = git_get_current_hash(llvm_repo_dir)
                    if last_build_hash == current_hash:
                        return LLVMStatus.up_to_date
        return LLVMStatus.needs_build
    return LLVMStatus.error


def llvm_save_last_build_hash() -> bool:
    if is_git_repo(llvm_repo_dir):
        if not git_has_uncommitted_changes(llvm_repo_dir):
            with open(llvm_last_build_hash_file, 'wb') as f:
                f.write(git_get_current_hash(llvm_repo_dir))
                return True
    return False


if os.path.exists(llvm_repo_dir):
    print("Directory '" + llvm_repo_dir + "' does already exist. Checking for git updates...")
    if not git_update(llvm_repo_dir):
        print('The directory seems to not contain a git repository, or the remote is missing.', file=sys.stderr)
        exit(1)
    status = git_remote_status(llvm_repo_dir)

    if status == GitRemoteStatus.error:
        print('Something went wrong while checking for git updates.', file=sys.stderr)
        exit(1)
    elif status == GitRemoteStatus.equal:
        print('The llvm repository is up-to-date with the upstream!')
    elif status == GitRemoteStatus.remote_ahead:
        print('The upstream has changes, should llvm be updated automatically (y/n)?', end=' ')
        answer = getch()
        while answer.lower() != 'y' and answer.lower() != 'n':
            answer = getch()

        print('\n')

        if answer.lower() == 'y':
            print('Pulling git updates...')

            git_pull(llvm_repo_dir)

    elif status == GitRemoteStatus.local_ahead:
        print('The local repository has unpushed changes and is ahead of the upstream.')
    else:
        print('Local repository and upstream have diverged. Consider merging the upstream manually.')
        print('Should the build process continue (y/n)?', end=' ')
        answer = getch()
        while answer.lower() != 'y' and answer.lower() != 'n':
            answer = getch()

        print('\n')

        if answer.lower() == 'n':
            print('You can run the build_script again when you are done merging the upstream of llvm.')
            exit(0)
else:
    print("Directory '" + llvm_repo_dir + "' wasn't found at the juice root! Cloning from github...")
    if not git_clone('https://github.com/juice-lang/juice-llvm.git', llvm_repo_dir):
        print('Something went wrong while cloning the llvm repository.', file=sys.stderr)
        exit(1)

if os.path.exists(llvm_build_dir) and args.should_rebuild_llvm:
    shutil.rmtree(llvm_build_dir)

os.makedirs(llvm_install_dir, exist_ok=True)

status = llvm_status()

if status == LLVMStatus.error:
    print('Something went wrong while checking, if llvm libraries need building.', file=sys.stderr)
    exit(1)
elif status == LLVMStatus.needs_build:
    print('Building the llvm libraries...')

    if not cmake(llvm_repo_dir, llvm_install_dir, llvm_build_dir):
        print('Something went wrong while generating the llvm Makefiles.', file=sys.stderr)
        exit(1)

    make_and_install(llvm_build_dir)
    llvm_save_last_build_hash()
else:
    print('The llvm libraries are up-to-date and therefore not rebuilt.')

if args.should_rebuild:
    shutil.rmtree(build_dir)
    os.makedirs(cmake_dir)
    os.makedirs(bin_dir)

if not cmake(repo_dir, build_dir, cmake_dir):
    print('Something went wrong while generating the llvm Makefiles.', file=sys.stderr)
    exit(1)

make_and_install(cmake_dir)
