# util/packages/progress/progress.py - Progress class, displays a simple progress bar
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2021 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

from __future__ import absolute_import, division, unicode_literals

import shutil

from timeit import default_timer as timer

from ..io import Color, background_colored, bold

__all__ = [
    'Progress'
]


class Progress:
    def __init__(self, pretty_output: bool):
        self.pretty_output = pretty_output

        self.start_time = 0.0

    def start(self):
        self.start_time = timer()

    def update(self, percent: int):
        elapsed_time = timer() - self.start_time

        columns = self._get_terminal_columns()

        line = ''

        prefix = ' {:>3}%'.format(percent)
        postfix = self._duration_string(elapsed_time) + ' '

        bar_start = ' ['
        bar_end = '] '

        bar_size = columns - len(prefix + bar_start + bar_end + postfix)
        amount = int(float(percent) / (100.0 / float(bar_size)))
        remain = bar_size - amount

        if remain < 0:
            if columns > len(prefix):
                line = prefix
        else:
            bar = self._highlight_done(amount) + self._highlight_todo(remain)

            if self.pretty_output:
                prefix = bold(prefix)

            line = prefix + bar_start + bar + bar_end + postfix

        if self.pretty_output:
            print('\r' + line, end='', flush=True)
        else:
            print(line)

    @staticmethod
    def _duration_string(duration: float) -> str:
        milliseconds = int(duration * 1000) % 1000
        seconds = int(duration) % 60
        minutes = int(duration / 60) % 60
        hours = int(duration / (60 * 60))

        if hours > 0:
            return '{}:{:0>2}:{:0>2}.{:0>3}'.format(hours, minutes, seconds,
                                                    milliseconds)
        return '{:0>2}:{:0>2}.{:0>3}'.format(minutes, seconds, milliseconds)

    def _highlight_done(self, repeat: int) -> str:
        if self.pretty_output:
            return background_colored(' ' * repeat, Color.yellow)
        return '=' * repeat

    def _highlight_todo(self, repeat: int) -> str:
        if self.pretty_output:
            return background_colored(' ' * repeat, Color.white)
        return ' ' * repeat

    @staticmethod
    def _get_terminal_columns() -> int:
        columns, _ = shutil.get_terminal_size((80, 20))
        return columns
