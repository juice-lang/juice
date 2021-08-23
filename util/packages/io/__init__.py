# util/packages/io/__init__.py - Package file of the io package
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2021 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

"""
This package provides some functions and classes for managing terminal output.
"""

from __future__ import absolute_import, unicode_literals

from .diagnostics import action, error, note, output, success, question

from .styles import should_style_output
from .styles import Color, ColorCombination, Style
from .styles import background_colored, bold, colored

__all__ = [
    'action',
    'error',
    'note',
    'output',
    'success',
    'question',


    'should_style_output',

    'Color',
    'ColorCombination',
    'Style',

    'background_colored',
    'bold',
    'colored'
]
