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

import subprocess
import argparse
import os
import shutil
from concurrent.futures import ThreadPoolExecutor, as_completed
from tqdm import tqdm
from os.path import isdir, join, basename, normpath

NUM_WORKERS = 50


def future_main(bin, input_file, output_file):
    with open(output_file, 'w') as f:
        subprocess.run([bin, input_file], stdout=f, stderr=subprocess.STDOUT)
    return 1

def main():
    parser = argparse.ArgumentParser(description='Read .csv files (local \
                                     orderings) from <input_dir> and run binary \
                                     with those .csv files as input. The \
                                     program will automatically create directory')
    parser.add_argument('ordering_bin', type=str, help='The binary that orders \
                        local orderings in <input_dir>. ')
    parser.add_argument('input_dir', type=str, help='The directory where local \
                        orderings are located. ')                                
    parser.add_argument('output_dir', type=str, help='The directory where output \
                        is stored. This python script will create \
                        <output_dir>/<input_dir>. This program exists if \
                        <output_dir>/<input_dir> already exists.')
    parser.add_argument('-p', '--path_bin', type=str, help='', default='')

    args = parser.parse_args()
    order_bin = args.ordering_bin
    input_dir = normpath(args.input_dir)
    output_dir = join(args.output_dir, basename(input_dir))
    path_bin = args.path_bin

    print('ordering binary: ', order_bin)
    print('input directory: ', input_dir)
    print('output directory: ', output_dir)
    print('path_bin: ', path_bin)
    print('num workders: ', NUM_WORKERS)

    if not isdir(input_dir):
        print(f'{input_dir} does not exist, exiting...')
        return
    if isdir(output_dir):
        print(f'{output_dir} already exist, exiting...')
        return
    else:
        os.makedirs(output_dir)
    
    futures = []
    executor = ThreadPoolExecutor(max_workers=NUM_WORKERS)
    dirs = os.listdir(input_dir)
    dirs.sort()
    for file in dirs:
        in_sub_dir = join(input_dir, file)
        if not isdir(in_sub_dir): continue
        out_sub_dir = join(output_dir, basename(in_sub_dir).zfill(4))
        os.makedirs(out_sub_dir)
        bin_inputs = os.listdir(in_sub_dir)
        print(f'input file dir: {in_sub_dir}, total {len(bin_inputs)} files')
        for bin_input in bin_inputs:
            input_file = join(in_sub_dir, bin_input)
            output_file = join(out_sub_dir, bin_input.split('.')[0]+'.log')
            futures.append(executor.submit(future_main, order_bin, input_file, output_file))

    if path_bin != '' and os.path.isfile(path_bin):
        try:
            with open(join(output_dir, 'all_paths.log'), 'w') as f:
                subprocess.run([path_bin, join(input_dir, 'template.csv')], stdout=f, stderr=subprocess.STDOUT)
        except: print(f'error running path find binary {path_bin}')

    if os.path.isfile(join(input_dir, 'README.md')):
        shutil.copy(join(input_dir, 'README.md'), join(output_dir, 'README.md'))

    with tqdm(total=len(futures)) as pbar:
        for future in as_completed(futures):
            future.result()
            pbar.update(1)


if __name__ == '__main__' :
    main()
