#!/usr/bin/env python
## Copyright (c) 2016 by Sony Computer Entertainment, Inc.
## This file is subject to the terms and conditions defined in file
## 'LICENSE.txt', which is part of this source code package.

# System modules
import argparse
import logging
import math
import sys

# Local modules
import files

logger = logging.getLogger (__name__)
logger.addHandler (logging.NullHandler ())


def statistics (samples):

    # Compute the population variance:
    #   1. Find the mean of the set of data.
    #   2. Subtract each number from the mean.
    #   3. Square the result.
    #   4. Add the results together.
    #   5. Divide the result by the total number of numbers in the data set.
    n = len (samples)
    mean = math.fsum (samples) / n
    variance = ((math.fsum ((x - mean) ** 2 for x in samples) / n) if n > 1 else 0)
    assert variance >= 0

    #std_deviation = math.sqrt (variance)
    #std_error = std_deviation / math.sqrt (n)
    return {
        'mean': mean,
        #'se'  : std_error, # value is not actually needed 
        'var' : variance
    }


def filter_names (key, times):
    return [ t [key] for t in times if t is not None ]

    
def merge (gathered):
    """
    Merges a list of "extract" dictionaries.
    The keys of a extract dictionary are the names of each of the parsed files; the
    value of each is the corresponding complete parse dictionary.
    """

    result = list ()

    # Build the set of object file names from those in the inputs. 'gathered'
    # is a list of two-tuples (json file, parse dictionary). The parse dictionary
    # keys are the object file names; values are the total and clang times.
    all_object_names = set (obj_name for g in gathered for obj_name in g [1])

    # Now iterate through the object file names, producing the statistics for
    # each file.
    for obj in all_object_names:
        # Gather all of the timings the object file 'obj' into the
        # 'times' list.
        times = list ()
        logger.debug ('Getting results for "%s"', obj)
        for g in gathered:
            json_file, timings = g
            if obj not in timings:
                logger.warning ('File "%s" was not found in BZ2/JSON file "%s"', obj, json_file)
            else:
                times.append (timings [obj])

        result.append ({
            'obj': obj,
            'total': statistics ([ t ['total'] for t in times ]),
            'clang': statistics ([ t ['clang'] for t in times ])
        })
    return result


def main (args = sys.argv [1:]):
    parser = argparse.ArgumentParser (description='Compute average from multiple test runs.')
    parser.add_argument ('infile', nargs='*',
                         help='The parsed (bz2/json) files to be merged.')
    parser.add_argument ('-o', '--output',
                         default='-',
                         metavar='F',
                         dest='outfile',
                         help='The file to which output will be written (default=stdout)')
    parser.add_argument ('-v', '--verbose', action='store_true',
                         help='Produce verbose output')
    options = parser.parse_args (args)

    # Set the root logger's level: this allows logging messages to
    # be logged to the default console.
    logging.getLogger ().setLevel (logging.DEBUG if options.verbose else logging.WARNING)

    logger.info ('Source files are: %s', str (options.infile))

    inputs = [ (infile, files.load_file (infile)) for infile in options.infile ]
    result = merge (inputs)
    files.write_json (result, options.outfile)
    return 0


if __name__ == '__main__':
    logging.basicConfig (level=logging.DEBUG)
    sys.exit (main ())

#eof merge.py
