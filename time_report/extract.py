#!/usr/bin/env python
## Copyright (c) 2016 by Sony Computer Entertainment, Inc.
## This file is subject to the terms and conditions defined in file
## 'LICENSE.txt', which is part of this source code package.

# System modules
import argparse
import logging
import sys

# Local modules
import files

logger = logging.getLogger (__name__)
logger.addHandler (logging.NullHandler ())

EXIT_SUCCESS = 0
EXIT_FAILURE = 1

CATEGORY = 'Miscellaneous Ungrouped Timers'
TOTAL = 'Total'
CODE_GENERATION = 'Code Generation Time'

TIME_KEY = 'user+system'

def extract (input_dict):
    result = dict ()
    logger.info ('Extracting %d items', len (input_dict))
    for name, tree in input_dict.items ():
        cg    = tree [CATEGORY][CODE_GENERATION].get (TIME_KEY, 0.0)
        total = tree [CATEGORY][TOTAL          ].get (TIME_KEY, 0.0)
        assert cg >= 0.0 and total >= 0.0 and cg <= total
        clang = total - cg
        result [name] = { 'total': total, 'clang': clang, }
        #print (name, result [name])
        #print (clang / total if total > 0.0 else 0.0)
    return result


def main (args = sys.argv [1:]):
    parser = argparse.ArgumentParser (description='Report front-end time from zip')
    parser.add_argument ('infile',
                         help='The name of the zip file to be opened')
    parser.add_argument ('-o', '--output',
                         default='-',
                         metavar='F',
                         dest='outfile',
                         help='The file to which output will be written (default=stdout)')
    parser.add_argument ('-v', '--verbose', action='store_true',
                         help='Produce verbose output')
    options = parser.parse_args (args)

    # Set the root logger's level to DEBUG: this allows all logging messages to
    # be logged somewhere (to the default console or file).
    logging.getLogger ().setLevel (logging.DEBUG if options.verbose else logging.WARNING)

    logger.info ('Source file is: "%s"', options.infile)

    input_dict = files.load_file (options.infile)
    result = extract (input_dict)
    files.write_json (result, options.outfile)
    return EXIT_SUCCESS


if __name__ == '__main__':
    logging.basicConfig (level=logging.DEBUG)
    sys.exit (main ())

#eof main.py
