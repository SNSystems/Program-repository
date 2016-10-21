#!/usr/bin/env python
## Copyright (c) 2016 by Sony Computer Entertainment, Inc.
## This file is subject to the terms and conditions defined in file
## 'LICENSE.txt', which is part of this source code package.

from __future__ import print_function
import argparse
import logging
import math
import os
import sys

# Local modules
import files

logger = logging.getLogger (__name__)
logger.addHandler (logging.NullHandler ())

EXIT_SUCCESS = 0
EXIT_FAILURE = 1

def mean (values):
    return math.fsum (values) / len (values)


def standard_error (variances):
    """
    Average the variances; then you can take square root to get the average
    standard deviation and from that derive the standard error.
    """

    m = mean (variances)
    std_deviation = math.sqrt (m)
    std_error = std_deviation / math.sqrt (len (variances))
    return std_error


def values_from_subsample (subsample, key):
    return [
        mean           ([ v [key]['mean'] for v in subsample ]),
        standard_error ([ v [key]['var' ] for v in subsample ])
    ]

def cg_ratio (total_mean, clang_mean):
    assert total_mean >= 0.0 and clang_mean >= 0.0 and clang_mean <= total_mean
    return (clang_mean / total_mean) if total_mean > 0.0 else 0.0


def output (outfile_name, num_samples_out, min_threshold, result):
    logger.info ('Output. Sorting...')
    result.sort (key = lambda r: cg_ratio (r ['total']['mean'], r ['clang']['mean']))
    result = [ r for r in result if r['total']['mean'] >= min_threshold ]

    logger.info ('Downsampling %d records...', len (result))

    record_count = 0
    temp_file_name = outfile_name + '.t'
    logger.info ('Writing to temporary file "%s"', temp_file_name)
    with open (temp_file_name, 'w') as outfile:
        outfile.write ('total total_error clang clang_error ratio\n')

        samples_per_group = max (math.trunc (len (result) / num_samples_out + 1), 1)
        logger.info ('%d samples per bin', samples_per_group)
    
        start_index = 0
        end_index = samples_per_group
        while start_index < len (result):
            subsample = result [start_index:end_index]
    
            record_count += 1
            total = values_from_subsample (subsample, 'total')
            clang = values_from_subsample (subsample, 'clang')
            outfile.write ('{total[0]:.4f} {total[1]:.4f} {clang[0]:.4f} {clang[1]:.4f} {ratio:f}\n'
                            .format (total=total, clang=clang, ratio=cg_ratio (total[0], clang[0])))
    
            start_index = end_index
            end_index += samples_per_group
        logger.info ('Wrote %d records', record_count)
    logger.info ('Renaming temp file "%s" to final name "%s"', temp_file_name, outfile_name)
    os.rename (temp_file_name, outfile_name)


def main (args = sys.argv [1:]):
    parser = argparse.ArgumentParser (description='Report front-end time from gather')
    parser.add_argument ('infile',
                         help='The name of the parse files to be opened')
    parser.add_argument ('-o', '--output',
                         default='-',
                         metavar='F',
                         dest='outfile',
                         help='The file to which output will be written (default=stdout)')
    parser.add_argument ('-t', '--min-threshold',
                         default=0,
                         type=float,
                         help='The time below which results will be discarded because the percentage error is too great')
    parser.add_argument ('-s', '--samples',
                         type=int, default=200,
                         metavar='N',
                         dest='num_samples_out',
                         help='The number of samples to be produced')
    parser.add_argument ('-v', '--verbose', action='store_true',
                         help='Produce verbose output')
    options = parser.parse_args (args)

    # Set the root logger's level to DEBUG: this allows all logging messages to
    # be logged somewhere (to the default console or file).
    logging.getLogger ().setLevel (logging.DEBUG if options.verbose else logging.WARNING)

    logger.info ('BZ2 source file is %s', options.infile)

    input = files.load_file (options.infile)
    output (options.outfile, options.num_samples_out, options.min_threshold, input)
    return EXIT_SUCCESS


if __name__ == '__main__':
    logging.basicConfig (level=logging.DEBUG)
    sys.exit (main ())

#eof convert.py
