# util/packages/io/styles.py - Classes for ANSI-colored output
#
# This file is part of the juice open source project
#
# Copyright (c) 2019 - 2021 juice project authors
# Licensed under MIT License
#
# See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
# See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

from __future__ import absolute_import, unicode_literals

from enum import Enum
from typing import Optional, Union

__all__ = [
    'should_style_output',

    'Color',
    'ColorCombination',
    'Style',

    'background_colored',
    'bold',
    'colored'
]


_should_style_output = True


class Style(Enum):
    reset = 0
    bold = 1

    def __str__(self) -> str:
        return '\033[{}m'.format(self.value) if _should_style_output else ''

    def __add__(self, other: Union[str, 'Color', 'ColorCombination']) -> str:
        if isinstance(other, str):
            return str(self) + other
        elif isinstance(other, Color):
            return self._add_implementation(other, False)
        elif isinstance(other, ColorCombination):
            return self._add_implementation(other, True)

        return NotImplemented

    def __radd__(self, other: Union[str, 'Color', 'ColorCombination']) -> str:
        if isinstance(other, str):
            return other + str(self)
        elif isinstance(other, Color):
            return self._add_implementation(other, False)
        elif isinstance(other, ColorCombination):
            return self._add_implementation(other, True)

        return NotImplemented

    def _add_implementation(self, other: Union['Color', 'ColorCombination'],
                            is_combination: bool) -> str:
        if self == self.bold:
            if not _should_style_output:
                return ''

            return '\033[3{};4{};1m'.format(other.fg_color.value,
                                            other.bg_color.value)\
                if is_combination else '\033[3{};1m'.format(other.value)

        return NotImplemented


class Color(Enum):
    black = 0
    red = 1
    green = 2
    yellow = 3
    blue = 4
    magenta = 5
    cyan = 6
    white = 7
    default = 9

    def __str__(self) -> str:
        return '\033[3{}m'.format(self.value) if _should_style_output else ''

    def __add__(self, other: Union[str, Style]) -> str:
        if isinstance(other, str):
            return str(self) + other
        elif isinstance(other, Style):
            return self._add_implementation(other)
        return NotImplemented

    def __radd__(self, other: Union[str, Style]) -> str:
        if isinstance(other, str):
            return other + str(self)
        elif isinstance(other, Style):
            return self._add_implementation(other)

        return NotImplemented

    def __and__(self, other: 'Color') -> 'ColorCombination':
        return ColorCombination(self, other)

    def _add_implementation(self, other: Style) -> str:
        if other == Style.bold:
            if not _should_style_output:
                return ''

            return '\033[3{};1m'.format(self.value)

        return NotImplemented


class ColorCombination:
    def __init__(self, fg_color: Color, bg_color: Color):
        self.fg_color = fg_color
        self.bg_color = bg_color

    def __str__(self) -> str:
        return '\033[3{};4{}m'.format(self.fg_color.value,
                                      self.bg_color.value)\
            if _should_style_output else ''

    def __add__(self, other: Union[str, Style]) -> str:
        if isinstance(other, str):
            return str(self) + other
        elif isinstance(other, Style):
            return self._add_implementation(other)

        return NotImplemented

    def __radd__(self, other: Union[str, Style]) -> str:
        if isinstance(other, str):
            return other + str(self)
        elif isinstance(other, Style):
            return self._add_implementation(other)

        return NotImplemented

    def _add_implementation(self, other: Style) -> str:
        if other == Style.bold:
            if not _should_style_output:
                return ''

            return '\033[3{};4{};1m'.format(self.fg_color.value,
                                            self.bg_color.value)

        return NotImplemented


def colored(string: str, fg_color: Color,
            bg_color: Optional[Color] = None) -> str:
    if bg_color is None:
        return fg_color + string + Style.reset
    return (fg_color & bg_color) + string + Style.reset


def background_colored(string: str, bg_color: Color) -> str:
    return (Color.default & bg_color) + string + Style.reset


def bold(string: str) -> str:
    return Style.bold + string + Style.reset


def should_style_output(new_value: bool):
    global _should_style_output
    _should_style_output = new_value
