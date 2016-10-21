#!/usr/bin/env python
## Copyright (c) 2016 by Sony Computer Entertainment, Inc.
## This file is subject to the terms and conditions defined in file
## 'LICENSE.txt', which is part of this source code package.

# System modules
import argparse
import codecs
import logging
import sys
import tarfile

# Local modules
import files
import time_report_parser

EXIT_SUCCESS = 0
EXIT_FAILURE = 1

logger = logging.getLogger (__name__)
logger.addHandler (logging.NullHandler ())

def parse (infile):
    logger.info ('Opening compressed tar file "%s"', infile)
    tar = tarfile.open (name=infile, mode='r:bz2')

    num_rows = lambda tree: sum ([ len (category.keys ()) for category in tree.values () ])
    result = dict ()
    members = tar.getmembers ()
    for member in members:
        source = tar.extractfile (member)
        reader = codecs.getreader ('utf-8')

        logger.info ("File {count} of {total} '{filename}'"
                     .format (count = len (result) + 1,
                              total = len (members),
                              filename = member.name))

        tree = time_report_parser.parse_file (reader (source))
        result [member.name] = tree

        logger.info ("Parsed {num_categories} categories, {num_results} results"
                     .format (num_categories=len (tree.keys ()), num_results = num_rows (tree)))
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
    # be logged somewhere.
    logging.getLogger ().setLevel (logging.DEBUG if options.verbose else logging.WARNING)

    logger.info ('Source file is: "%s"', options.infile)
    result = parse (options.infile)
    files.write_json (result, options.outfile)
    return EXIT_SUCCESS


if __name__ == '__main__':
    logging.basicConfig (level=logging.DEBUG)
    sys.exit (main ())

#eof extract.py
