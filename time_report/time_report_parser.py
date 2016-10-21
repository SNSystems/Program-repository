#!/usr/bin/env python
## Copyright (c) 2016 by Sony Computer Entertainment, Inc.
## This file is subject to the terms and conditions defined in file
## 'LICENSE.txt', which is part of this source code package.
'''
A parser for clang -ftime-report output.

The input is a file or string in a format which roughly resembles the following:

    ===-----------===
         group 1
    ===-----------===
    --column 1--   --column 2--   --name--
       0.1 ( 2%)      0.2 ( 3%)   foo
       0.5 ( 7%)      1.1 (13%)   bar
       1.7 (19%)      2.3 (23%)   foo
    ===-----------===
         group 2
    ===-----------===
    --column 1--   --column 2--   --name--
       2.9 (31%)      3.7 (41%)   qaz
       0.1 ( 2%)      0.3 ( 5%)   zaq

Returns a nested dictionary of categories/rows/values so that input above would
be represented as:

    {
        'group 1': {
            'foo': { 'column 1': 1.8, 'column 2': 2.5 },
            'bar': { 'column 1': 0.5, 'column 2': 1.1 }
        },
        'group 2': {
            'qaz': { 'column 1': 2.9, 'column 2': 3.7 },
            'zaq': { 'column 1': 0.1, 'column 2': 0.3 }
        }
    }

Identically named rows within a category are aggregated so that there is a
single entry holding the total of all the times recorded for that name.
'''

from __future__ import print_function
from collections import Counter
import argparse
import json
import pyparsing as pp
import sys


_EOL = pp.LineEnd ().suppress ()
printing_chars = pp.Word (pp.printables).setWhitespaceChars (' \t')


#@pp.traceParseAction
def _as_number (toks):
    '''Converts a token representing a number and converts it to an int or float as appropriate.'''
    assert len (toks) == 1
    try:
        return int (toks [0])
    except ValueError:
        return float (toks [0])


# Matches a number which may be negative or include a decimal point.
number = pp.Combine (pp.Optional ('-') # allow negative numbers(?)
                     + pp.Word (pp.nums)
                     + pp.Optional ('.' + pp.Word (pp.nums))
                     ).setParseAction (_as_number)


# Matches a line containing '===---===' (where the number of '=' and '-' is
# unknown). These appear before and after the line containing the category
# title.
category_title_separator = (pp.LineStart () + pp.Regex ('=+-+=+')).suppress () + _EOL

category_title = (category_title_separator
    + pp.originalTextFor (pp.OneOrMore (printing_chars)) + _EOL
    + category_title_separator)
#category_title.setDebug (True)


# Matching for something like: "  Total Execution Time: d.dddd seconds (d.dddd wall clock)"
total_time = (pp.CaselessLiteral ('Total Execution Time:') 
    + number + 'seconds'
    + '(' + number + 'wall' + 'clock' + ')'
    + _EOL)


# ----
# This group of patterns matches a table header row; that is, a row which resembles:
#    ---User Time---   --System Time-- ...
dashes = pp.Regex (r'-+').suppress ()
# Matches a name with potentially embedded whitespace and ending with a dash.
# The parse action removes leading or trailing whitespace and converts the
# result to lower-case
column_name = (
    pp.Word (pp.printables.replace ('-','') + ' ')
           .setWhitespaceChars (' \t')
           .setParseAction (lambda t: t[0].strip ().lower ())
)
header = pp.Group (pp.OneOrMore (dashes + column_name + dashes))
# ----



# ----
# This group of patterns matches a row of results; that is, something like:
#   0.1 (1%)  0.2 (2%)  row-name

# Matches "(xx.x%)"
percentage = pp.Literal ('(') + number + '%' + ')'
# A 'row_number' is a number with an optional percentage (which we ignore):
row_number = number + pp.Optional (percentage).suppress ()
row_dashes = pp.Literal ('-----').setParseAction (lambda t: 0)
name = pp.Word (pp.printables + ' ').setParseAction (lambda t: t [0].strip ())

# A row is a series of values (with their optional percentage) followed by a
# name string and the end-of-line. Sometimes it's a series of dashes which
# are emitted for values < 1e-7; in this case, we return 0.
row = pp.Group (pp.OneOrMore (row_number ^ row_dashes) + name) + _EOL
# ----


# Match a group of rows.
rows = pp.Group (pp.OneOrMore (row))


#@pp.traceParseAction
def _one_category_pa (toks):
    '''A parse-action which is used to convert a group-name, header, and list of values a
    two-tuple with the group-name and table contents as a dictionary (the keys of which are
    the row names and the values are another dictionary).

    On input, the toks array contains:
    [0]: The group name (a "group" being a string with "===---===" lines preceeding and
         following it).
    [1]: An list of column names
    [2]: An list of lists of values. The elements of the inner list correspond to a single
         number; there's an element in the outer list for each row. The length of the inner
         list should be the same as the length of the tok[1] list.

    In input file of the form:
        ===-----------====
           group name
        ===-----------====
        --col1-- --col2--  --name--
             0.1      0.2  foo
             0.3      0.4  bar

    Will, on input, be represented in the 'toks[]' list as:
        toks = [
            'group name',
            [ 'col1', 'col2', 'name' ],
            [ [ 0.1, 0.2, 'foo' ], [ 0.3, 0.4, 'bar' ] ]
        ]

    Returns:
        ( 'group name', [
            { 'col1': 0.1, 'col2': 0.2, 'name': 'foo' },
            { 'col1': 0.3, 'col2': 0.4, 'name': 'bar' }
        ] )
    '''

    assert len (toks) == 3
    group_name, column_names, rows = toks
    content = [ dict (zip (column_names, r)) for r in rows ]
    return (group_name, content)


# Matches an entire results category; something like:
#        ===-----------====
#           group name
#        ===-----------====
#        --col1-- --col2--  --name--
#             0.1      0.2  foo
#             0.3      0.4  bar
# See _one_category_pa() for a description of the resulting tokens.
category = ((category_title
             + pp.Optional (total_time.suppress ())
                            + header
                            + rows)).setParseAction (_one_category_pa)


# Consumes any unknown lines of text between categories
unknown = (pp.ZeroOrMore (pp.White ())
           + pp.OneOrMore (printing_chars)
           + _EOL)


#@pp.traceParseAction
def _categories_pa (toks):
    '''In input file of the form:
        ===-----------====
             group1
        ===-----------====
        --c1-- --c2--  --name--
           0.1    0.2  foo
           0.3    0.5  bar
           0.7    1.1  foo
        ===-----------====
             group2
        ===-----------====
        --c1-- --c2--  --name--
           1.3    1.7  qaz
           1.9    2.3  zaq

    Will be represented in the 'toks[]' list as:
        toks = [
            ('group1', [
                { 'c1':0.1, 'c2':0.2, 'name':'foo' },
                { 'c1':0.3, 'c2':0.5, 'name':'bar' },
                { 'c1':0.7, 'c2':1.1, 'name':'foo' },
            ]),
            ('group2', [
                { 'c1':1.3, 'c2':1.7, 'name':'qaz' },
                { 'c1':1.9, 'c2':2.3, 'name':'zaq' },
            ])
        ]

    Returns:
        {
            'group1': {
                'foo': { 'c1': 0.8, 'c2':1.3 },
                'bar': { 'c1': 0.3, 'c2':0.5 },
            },
            'group2': {
                'qaz': { 'c1': 1.3, 'c2':1.7 },
                'zaq': { 'c1': 1.9, 'c2':2.3 },
            }
        }
    Note that rows with the same name in a category are aggregated (such as group1/foo
    in the example above).
    '''

    categories = dict ()
    # On input, toks[] is a list of two-tuples.
    for cat_name, cat_content in toks:
        for row in cat_content:
            row_name = row ['name']
            del row ['name']

            # Creates (if needed) or uses an existing category name entry in the
            # 'categories' dictionary then creates or uses an existing row name
            # entry in that dictionary. The current row's values are then added
            # to the existing Counter instance.
            (categories.setdefault (cat_name, dict ())
                       .setdefault (row_name, Counter ())
                       .update (row))
    return categories


# Grammar concatenates a series of "unknown" lines and parsed categories.
# (Removing 'unknown' disables the skipping of unrecognized lines. That'll make the parse
# more robust becuase the program will no longer silently swallow any lines that it doesn't
# match.)
grammar = pp.OneOrMore (#unknown.suppress () ^ 
                        category
                        ^ _EOL).setParseAction (_categories_pa)


def parse_string (s):
    '''Parses a clang -ftime-report contained in the string 's'.
    Returns a nested dictionary of categories/rows/values.
    '''

    return grammar.parseString (s, parseAll=True) [0]


def parse_file (file_or_filename):
    '''Parses a clang -ftime-report contained in the supplied file-like
    object or filename.

    If a filename is specified (instead of a file object), the entire file is
    opened, read, and closed before parsing. Returns a nested dictionary of
    categories/rows/values.
    '''

    try:
        file_contents = file_or_filename.read ()
    except AttributeError:
        with open (file_or_filename, 'rt') as f:
            file_contents = f.read ()
    return parse_string (file_contents)



def main (args = sys.argv [1:]):
    '''Implements a command-line utility which will parse the specified file and
    return a JSON-formatted string containing the results.
    '''

    parser = argparse.ArgumentParser (description='Parse clang -ftime-report')
    parser.add_argument ('infile', type=argparse.FileType ('r'),
                         help='The name of the file to be parsed')
    options = parser.parse_args (args)
    result = parse_file (options.infile)
    return json.dumps (result, sort_keys=True, indent=4)


if __name__ == '__main__':
    json_result = main ()
    print (json_result)
        
#eof report2.py
