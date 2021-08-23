# util/packages/io/diagnostics.py - Functions for outputting diagnostics
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2021 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

from __future__ import absolute_import, unicode_literals

import sys

from typing import Callable

from .styles import Color, Style

__all__ = [
    'action',
    'error',
    'note',
    'output',
    'success',
    'question'
]


def _find_getch() -> Callable[[], str]:
    try:
        import termios
    except ImportError:
        # Non-POSIX. Return msvcrt's (Windows') getch.
        try:
            import msvcrt
            return msvcrt.getch
        except ImportError:
            return NotImplemented

    # POSIX system. Create and return a getch that manipulates the tty.
    import tty

    def _getch() -> str:
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


def output(text: str, color: Color, bold: bool = False, prefix: str = ' -> '):
    if bold:
        print(Style.bold + color + prefix + text + Style.reset)
    else:
        print(color + prefix + text + Style.reset)


def success(text: str, prefix: str = ' -> '):
    output(text, Color.green, bold=True, prefix=prefix)


def note(text: str, prefix: str = ' -> '):
    output(text, Color.blue, prefix=prefix)


def action(text: str, prefix: str = ' -> '):
    output(text, Color.cyan, prefix=prefix)


def error(text: str, prefix: str = ' error: '):
    output(text, Color.red, prefix=prefix)


def question(text: str, color: Color, prefix: str = ' -> ') -> bool:
    print(color + prefix + text + ' (y/n)?' + Style.reset, end=' ', flush=True)

    answer = getch()
    while answer.lower().strip() not in ['y', 'yes', 'n', 'no']:
        answer = getch()

    print('\n')

    return answer.lower().strip() in ['y', 'yes']
