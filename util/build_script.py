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


from __future__ import absolute_import, print_function, unicode_literals

import argparse
import os
import re
import selectors
import shutil
import subprocess
import sys

from enum import Enum
from typing import Optional


from packages.progress import Progress
from packages.io import action, error, note, success, question
from packages.io import Color, should_style_output


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


git_env = os.environ.copy()
git_env['LANG'] = 'en_US.UTF-8'

is_tty = sys.stdout.isatty()


def job_count() -> int:
    count = os.cpu_count()

    if count is not None:
        return count + 1

    return 3


def is_git_repo(cwd: str) -> bool:
    try:
        if cwd in is_git_repo.git_repos:
            return True
    except AttributeError:
        is_git_repo.git_repos = []

    with subprocess.Popen(['git', 'rev-parse'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          cwd=cwd, env=git_env,
                          encoding='utf-8') as git_process:
        while True:
            print(git_process.stdout.readline().strip())

            returncode = git_process.poll()

            if returncode is not None:
                if returncode == 0:
                    is_git_repo.git_repos.append(cwd)
                    return True
                return False


def git_clone(url: str, cwd: str) -> bool:
    with subprocess.Popen(['git', 'clone', url, cwd],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          env=git_env, encoding='utf-8') as clone_process:
        while True:
            print(clone_process.stdout.readline().strip())

            returncode = clone_process.poll()

            if returncode is not None:
                print('')
                return returncode == 0


def git_update(cwd: str) -> bool:
    if not is_git_repo(cwd):
        return False

    with subprocess.Popen(['git', 'remote', 'update'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          cwd=cwd, env=git_env,
                          encoding='utf-8') as update_process:
        while True:
            print(update_process.stdout.readline().strip())

            returncode = update_process.poll()

            if returncode is not None:
                print('')
                return returncode == 0


def git_remote_status(cwd: str) -> GitRemoteStatus:
    def get_git_hash(command: list) -> Optional[bytes]:
        process = subprocess.run(command,
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                 cwd=cwd, env=git_env)
        error_output = process.stderr.decode('utf-8').strip()
        if error_output != '':
            print(error_output + '\n')

        if process.returncode == 0:
            return process.stdout
        return None

    if is_git_repo(cwd):
        local_hash = get_git_hash(['git', 'rev-parse', '@'])
        if local_hash is None:
            return GitRemoteStatus.error

        remote_hash = get_git_hash(['git', 'rev-parse', '@{u}'])
        if remote_hash is None:
            return GitRemoteStatus.error

        base_hash = get_git_hash(['git', 'merge-base', '@', '@{u}'])
        if base_hash is None:
            return GitRemoteStatus.error

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
    if not is_git_repo(cwd):
        return False

    with subprocess.Popen(['git', 'pull'],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          cwd=cwd, env=git_env,
                          encoding='utf-8') as pull_process:
        while True:
            print(pull_process.stdout.readline().strip())

            returncode = pull_process.poll()

            if returncode is not None:
                print('')
                return returncode == 0


def git_checkout(branch: str, cwd: str) -> bool:
    if not is_git_repo(cwd):
        return False

    with subprocess.Popen(['git', 'checkout', branch],
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          cwd=cwd, env=git_env,
                          encoding='utf-8') as checkout_process:
        while True:
            print(checkout_process.stdout.readline().strip())

            returncode = checkout_process.poll()

            if returncode is not None:
                print('')
                return returncode == 0


def git_has_uncommitted_changes(cwd: str) -> bool:
    if is_git_repo(cwd):
        if subprocess.check_output(['git', 'status', '--porcelain'],
                                   cwd=cwd, env=git_env):
            return True
    return False


def git_get_current_hash(cwd: str) -> bytes:
    if is_git_repo(cwd):
        return subprocess.check_output(['git', 'rev-parse', 'HEAD'],
                                       cwd=cwd, env=git_env).strip()
    return b''


def git_is_branch(branch: str, cwd: str) -> bool:
    if is_git_repo(cwd):
        git_command = ['git', 'show-ref', '--verify', '-q',
                       'refs/heads/' + branch]

        git_process = subprocess.run(git_command, cwd=cwd, env=git_env)

        return git_process.returncode == 0

    return False


def git_get_current_branch(cwd: str) -> Optional[str]:
    if is_git_repo(cwd):
        git_command = ['git', 'symbolic-ref', '--short', '-q', 'HEAD']

        git_process = subprocess.run(git_command, stdout=subprocess.PIPE,
                                     cwd=cwd, env=git_env, encoding='utf-8')

        if git_process.returncode == 0:
            return git_process.stdout.strip()
    return None


def cmake(source_dir: str, install_dir: str, cwd: str) -> bool:
    action('Configuring build...\n')

    cmake_command = ['cmake', source_dir, '-G', 'Unix Makefiles',
                     '-D', 'CMAKE_INSTALL_PREFIX=' + install_dir,
                     '-D', 'CMAKE_BUILD_TYPE=' + args.build_type]

    if is_tty:
        cmake_command.extend(['-D', 'FORCE_COLORED_OUTPUT=TRUE'])

    with subprocess.Popen(cmake_command,
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          cwd=cwd, encoding='utf-8') as cmake_process:
        while True:
            print(cmake_process.stdout.readline().strip())

            returncode = cmake_process.poll()

            if returncode is not None:
                print('')
                return returncode == 0


def make_and_install(cwd: str) -> bool:
    action('Starting build...\n')

    progress = Progress(is_tty)

    with subprocess.Popen(['make', '-j', '{}'.format(job_count()), 'install'],
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                          cwd=cwd, encoding='utf-8') as make_process:
        progress.start()

        sel = selectors.DefaultSelector()
        sel.register(make_process.stdout, selectors.EVENT_READ)
        sel.register(make_process.stderr, selectors.EVENT_READ)

        while True:
            for key, _ in sel.select():
                if key.fileobj is make_process.stdout:
                    output = key.fileobj.readline().strip()
                    match = re.match(r'^\[\s*(\d+)%]', output)
                    if match is not None:
                        percent = int(match.group(1))
                        progress.update(percent)
                else:
                    output = key.fileobj.readline().strip()
                    if output != '':
                        print(output, flush=True)

            returncode = make_process.poll()

            if returncode is not None:
                print('\n')

                if returncode == 0:
                    success('Done!\n')
                    return True
                return False


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
                    help="don't auto-update LLVM before building")
parser.add_argument('--no-rebuild',
                    action='store_false',
                    dest='should_rebuild',
                    help="don't rebuild the juice project completely")
parser.add_argument('--rebuild-llvm',
                    action='store_true',
                    dest='should_rebuild_llvm',
                    help='force rebuilding of the LLVM libraries')
parser.add_argument('--llvm-branch',
                    default='juice/main',
                    dest='llvm_branch',
                    help='the branch of juice-llvm that should be used')

parser.set_defaults(build_type='Release')

args = parser.parse_args()


util_dir = os.path.dirname(os.path.realpath(__file__))
repo_dir = os.path.dirname(util_dir)
root_dir = os.path.dirname(repo_dir)

build_dir = os.path.join(repo_dir, 'build')
cmake_dir = os.path.join(build_dir, 'cmake')
bin_dir = os.path.join(build_dir, 'bin')

llvm_repo_dir = os.path.join(root_dir, 'juice-llvm')
llvm_llvm_dir = os.path.join(llvm_repo_dir, 'llvm')
llvm_build_dir = os.path.join(llvm_llvm_dir, 'build')
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


def main() -> int:
    if not is_tty:
        should_style_output(False)

    success('Building the juice compiler!\n', prefix='\n ')

    note('Building in ' + Color.magenta + args.build_type.upper()
         + Color.blue + ' mode.\n')

    action('Checking for LLVM repository...')

    if os.path.exists(llvm_repo_dir):
        note('Directory ' + Color.magenta + "'" + llvm_repo_dir + "'"
             + Color.blue + ' does already exist.\n')

        if args.should_update:
            action('Checking for git updates...\n')

            if not git_update(llvm_repo_dir):
                error('Something went wrong while checking for git updates.')
                return 1

            current_branch = git_get_current_branch(llvm_repo_dir)

            if current_branch is None or current_branch != args.llvm_branch:
                note('The LLVM repository is on the wrong branch.\n')
                action('Trying to check out '
                       + Color.magenta + "'" + args.llvm_branch + "'"
                       + Color.cyan + '...\n')

                if not git_is_branch(args.llvm_branch, llvm_repo_dir):
                    error('There is no branch called '
                          + Color.magenta + "'" + args.llvm_branch + "'"
                          + Color.red + ' in the juice-llvm repository.')
                    return 1

                if git_has_uncommitted_changes(llvm_repo_dir):
                    error('Could not check out the branch called '
                          + Color.magenta + "'" + args.llvm_branch + "'"
                          + Color.red
                          + ', because there are uncommitted changes.')
                    error('Commit the changes in the LLVM repository or pass in'
                          'the current branch using '
                          + Color.magenta + "'--llvm_branch'"
                          + Color.red + '.', prefix=' -> ')
                    return 1

                if not git_checkout(args.llvm_branch, llvm_repo_dir):
                    error('Something went wrong while checking out the branch.')
                    return 1

            status = git_remote_status(llvm_repo_dir)

            if status == GitRemoteStatus.error:
                error('Something went wrong while checking for git updates.')
                return 1
            elif status == GitRemoteStatus.equal:
                note('The LLVM repository is up-to-date with the upstream!')
            elif status == GitRemoteStatus.remote_ahead:
                if question('The upstream has changes, should LLVM be updated '
                            'automatically', Color.blue):
                    action('Pulling git updates...\n')

                    if not git_pull(llvm_repo_dir):
                        error('Something went wrong while pulling the git '
                              'updates.')
                        return 1

            elif status == GitRemoteStatus.local_ahead:
                note('The local repository has unpushed changes and is ahead '
                     'of the upstream.')
            else:
                note('Local repository and upstream have diverged. Consider '
                     'merging the upstream manually.')
                if not question('Should the build process continue',
                                Color.blue):
                    note('You can run '
                         + Color.magenta + os.path.basename(__file__)
                         + Color.blue + ' again when you are done merging the '
                         'upstream of LLVM.')
                    return 0
    else:
        note('Directory ' + Color.magenta + "'" + llvm_repo_dir + "'"
             + Color.blue + " wasn't found!\n")
        action('Cloning from github...\n')

        if not git_clone('https://github.com/juice-lang/juice-llvm.git',
                         llvm_repo_dir):
            error('Something went wrong while cloning the LLVM repository.')
            return 1

        action('Trying to check out '
               + Color.magenta + "'" + args.llvm_branch + "'"
               + Color.cyan + '...\n')

        if not git_is_branch(args.llvm_branch, llvm_repo_dir):
            error('There is no branch called '
                  + Color.magenta + "'" + args.llvm_branch + "'"
                  + Color.red + ' in the juice-llvm repository.')
            return 1

        if not git_checkout(args.llvm_branch, llvm_repo_dir):
            error('Something went wrong while checking out the branch '
                  + Color.magenta + "'" + args.llvm_branch + "'"
                  + Color.red + '.')
            return 1

    if os.path.exists(llvm_build_dir) and args.should_rebuild_llvm:
        shutil.rmtree(llvm_build_dir)

    os.makedirs(llvm_install_dir, exist_ok=True)

    status = llvm_status()

    if status == LLVMStatus.error:
        error('Something went wrong while checking, if the LLVM libraries need '
              'a rebuild.')
        return 1
    elif status == LLVMStatus.needs_build:
        note('The LLVM libraries need to be built.\n')
        action('Building the LLVM libraries...\n')

        if not cmake(llvm_llvm_dir, llvm_install_dir, llvm_build_dir):
            error('Something went wrong while generating the LLVM Makefiles.')
            return 1

        if not make_and_install(llvm_build_dir):
            error('Something went wrong while building the LLVM libraries.')
            return 1

        llvm_save_last_build_hash()
    else:
        note('The LLVM libraries are up-to-date and will therefore not be '
             'rebuilt.')

    if args.should_rebuild:
        shutil.rmtree(build_dir)
        os.makedirs(cmake_dir)
        os.makedirs(bin_dir)

    action('Building juice...\n', prefix='\n -> ')

    if not cmake(repo_dir, build_dir, cmake_dir):
        error('Something went wrong while generating the juice Makefiles.')
        return 1

    if not make_and_install(cmake_dir):
        error('Something went wrong while building the juice compiler.')
        return 1

    success('Finished building the juice compiler!\n')

    note('The executables have been installed under '
         + Color.magenta + "'" + bin_dir + "'" + Color.blue + '.\n')

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.exit(1)
