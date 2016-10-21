#!/usr/bin/env python
## Copyright (c) 2016 by Sony Computer Entertainment, Inc.
## This file is subject to the terms and conditions defined in file
## 'LICENSE.txt', which is part of this source code package.

import bz2
import codecs
import json
import logging
import os
import sys

logger = logging.getLogger (__name__)
logger.addHandler (logging.NullHandler ())

def write_json (result, outfile_name):
    logger.info ('Writing JSON result to "%s"', outfile_name)
    if outfile_name == '-':
        json.dump (result, sys.stdout, indent=2)
    else:
        temp_file_name = outfile_name + '.t'
        logger.info ('Writing to JSON temporary file "%s"', temp_file_name)
        with open (temp_file_name, 'w') as f:
            json.dump (result, f, indent=2)
        logger.info ('Renaming temp file "%s" to final name "%s"', temp_file_name, outfile_name)
        os.rename (temp_file_name, outfile_name)


def load_file (infile):
    logger.info ('Loading JSON from "%s"', infile)
    with open (infile) as f:
        return json.load (f)
#eof files.py
