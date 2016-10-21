#!/usr/bin/env python3

# System modules
import argparse
import csv
import json
import math
import os
import sys
# Local modules
import files

def standard_error (variance, num_samples):
    """
    Average the variances; then you can take square root to get the average
    standard deviation and from that derive the standard error.
    """

    std_deviation = math.sqrt (variance)
    std_error = std_deviation / math.sqrt (num_samples)
    return std_error


class Stats:
    def __init__ (self):
        self.__sum = 0
        self.__variance = 0

    def append (self, result):
        self.__sum += result ['mean']
        self.__variance += result ['var']

    def mean (self, samples):
        return self.__sum / samples

    def standard_error (self, samples):
        return standard_error (self.__variance, num_samples=samples)

    def sum (self):
        return self.__sum


def delta (r2, r1):
    assert r2 ['mean'] >= r1 ['mean']
    return {
        'mean': r2 ['mean'] - r1 ['mean'],
        'var' : r2 ['var'] - r1 ['var'],
    }

def main (args = sys.argv[1:]):
    parser = argparse.ArgumentParser (description='Produce total and error from .merge files.')
    parser.add_argument ('infile', nargs='+')
    parser.add_argument ('--config', default='config.json')
    options = parser.parse_args (args)

    with open (options.config) as fp:
        config = json.load (fp)
        
    output = csv.writer (sys.stdout)
    output.writerow ([
        'path',
        'samples',
        'clang sum', 'clang mean', 'clang se',
        'cg sum', 'cg mean', 'cg se'
    ])
    for path in options.infile:
        results = files.load_file (path)

        clang = Stats ()
        cg = Stats ()

        for result in results:
            clang.append (result ['clang'])
            cg.append (delta (result ['total'], result ['clang']))

        path = os.path.basename (path)
        path = os.path.splitext (path) [0]
        path = config.get (path, path.replace ('_', ' '))
        
        samples = len (results)
        output.writerow ([
            path,
            samples,
            clang.sum (),
            clang.mean (samples),
            clang.standard_error (samples),
            cg.sum () ,
            cg.mean (samples),
            cg.standard_error (samples)
        ]);

if __name__ == '__main__':
    main ()
