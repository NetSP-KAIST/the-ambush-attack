# GNU GENERAL PUBLIC LICENSE
# Version 3, 29 June 2007
# 
# Copyright (C) 2024 Ambush-attack-authors
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import os
import argparse
from os.path import join

UNSPOOL_RES_DIR = './data-unspool-wp'

def main():
    parser = argparse.ArgumentParser(description='Read unspooled results of Themis \
                                     and produces summary of it.')
    parser.add_argument('--dir', type=str, help='Input the directory that contains the \
                        results. For example, data-unspool-aws', default=UNSPOOL_RES_DIR)
    args = parser.parse_args()
    input_dir = args.dir
    dir = os.listdir(input_dir)
    dir.sort()
    print(f'dir: {dir}\n')
    for run in dir:
        run = join(input_dir, run)
        print(f'run: {run}')
        sub_dir = os.listdir(run)
        sub_dir.sort()
        for graph_point in sub_dir:
            graph_point = join(run, graph_point)
            if os.path.isdir(graph_point): 
                total_succ = 0
                total_trys = 0
                ordering_res = os.listdir(graph_point)
                ordering_res.sort()
                for one_ordering_res in ordering_res:
                    one_ordering_res = join(graph_point, one_ordering_res)
                    with open(one_ordering_res, 'r') as file:
                        for line in file:
                            if 'total' in line:
                                parts = line.split()
                                succ = int(parts[1])
                                trys = int(parts[3])
                                total_succ += succ
                                total_trys += trys
                if total_trys != 0:
                    print(graph_point, total_succ, total_trys, f'{float(total_succ/total_trys):.4f}')


if __name__ == '__main__':
    main()