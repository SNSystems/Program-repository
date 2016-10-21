#!/usr/bin/env python
## Copyright (c) 2016 by Sony Computer Entertainment, Inc.
## This file is subject to the terms and conditions defined in file
## 'LICENSE.txt', which is part of this source code package.

# System modules
import unittest

# Local modules
import time_report_parser


class test_time_report_parser (unittest.TestCase):

    def test_basic (self):
        actual = time_report_parser.parse_string ('''
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
''')
        expected = {
            'group 1': {
                'foo': { 'column 1': 1.8, 'column 2': 2.5, },
                'bar': { 'column 1': 0.5, 'column 2': 1.1 }
            },
            'group 2': {
                'qaz': { 'column 1': 2.9, 'column 2': 3.7 },
                'zaq': { 'column 1': 0.1, 'column 2': 0.3 }
            }
        }
        self.assertEqual (actual, expected)

    def test_row_dashes (self):
        input = '0.1 ( 2%) -----  name'
        actual = time_report_parser.row.parseString (input)
        expected = [[ 0.1, 0, 'name' ]]
        self.assertEqual (actual.asList (), expected)

if __name__ == '__main__':
    unittest.main ()

#eof test_time_report_parser.py
